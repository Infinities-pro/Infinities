// Copyright(C) 2023 InfiniFlow, Inc. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

module;

export module merge_knn;

import stl;

import knn_result_handler;

import infinity_exception;
import bitmask;
import default_values;
import internal_types;

namespace infinity {

export class MergeKnnBase {
public:
    virtual ~MergeKnnBase() = default;
};

export template <typename QueryElemType, template <typename, typename> typename C, typename DistType>
class MergeKnn final : public MergeKnnBase {
    using ResultHandler = HeapResultHandler<C<DistType, RowID>>;
    using DistFunc = DistType (*)(const QueryElemType *, const QueryElemType *, SizeT);

public:
    explicit MergeKnn(u64 query_count, u64 topk)
        : total_count_(0), query_count_(query_count), topk_(topk), idx_array_(MakeUniqueForOverwrite<RowID[]>(topk * query_count)),
          distance_array_(MakeUniqueForOverwrite<DistType[]>(topk * query_count)) {
        result_handler_ = MakeUnique<ResultHandler>(query_count, topk, this->distance_array_.get(), this->idx_array_.get());
    }

    ~MergeKnn() final = default;

public:
    void Search(const QueryElemType *query, const QueryElemType *data, u32 dim, DistFunc dist_f, u16 row_cnt, u32 segment_id, u16 block_id);

    void Search(const QueryElemType *query, const QueryElemType *data, u32 dim, DistFunc dist_f, u32 segment_id, u32 segment_offset);

    void Search(const QueryElemType *query, const QueryElemType *data, u32 dim, DistFunc dist_f, u16 row_cnt, u32 segment_id, u16 block_id, Bitmask &bitmask);

    void Search(const DistType *dist, const RowID *row_ids, u16 count);

    void Search(SizeT query_id, const DistType *dist, const RowID *row_ids, u16 count);

    void Begin();

    void End();

    void EndWithoutSort();

    DistType *GetDistances() const;

    RowID *GetIDs() const;

    DistType *GetDistancesByIdx(u64 idx) const;

    RowID *GetIDsByIdx(u64 idx) const;

    i64 total_count() const { return total_count_; }

private:
    i64 total_count_{};
    bool begin_{false};
    u64 query_count_{};
    i64 topk_{};
    UniquePtr<RowID[]> idx_array_{};
    UniquePtr<DistType[]> distance_array_{};

private:
    UniquePtr<ResultHandler> result_handler_{};
};

template <typename QueryElemType, template <typename, typename> typename C, typename DistType>
void MergeKnn<QueryElemType, C, DistType>::Search(const QueryElemType *query, const QueryElemType *data, u32 dim, DistFunc dist_f, u16 row_cnt, u32 segment_id, u16 block_id) {
    this->total_count_ += row_cnt;
    u32 segment_offset_start = block_id * DEFAULT_BLOCK_CAPACITY;
    for (u64 i = 0; i < this->query_count_; ++i) {
        const QueryElemType *x_i = query + i * dim;
        const QueryElemType *y_j = data;
        for (u16 j = 0; j < row_cnt; ++j, y_j += dim) {
            auto dist = dist_f(x_i, y_j, dim);
            result_handler_->AddResult(i, dist, RowID(segment_id, segment_offset_start + j));
        }
    }
}

template <typename QueryElemType, template <typename, typename> typename C, typename DistType>
void MergeKnn<QueryElemType, C, DistType>::Search(const QueryElemType *query, const QueryElemType *data, u32 dim, DistFunc dist_f, u32 segment_id, u32 segment_offset) {
    ++this->total_count_;
    for (u64 i = 0; i < this->query_count_; ++i) {
        const QueryElemType *x_i = query + i * dim;
        auto dist = dist_f(x_i, data, dim);
        result_handler_->AddResult(i, dist, RowID(segment_id, segment_offset));
    }
}

template <typename QueryElemType, template <typename, typename> typename C, typename DistType>
void MergeKnn<QueryElemType, C, DistType>::Search(const QueryElemType *query,
                                             const QueryElemType *data,
                                             u32 dim,
                                             DistFunc dist_f,
                                             u16 row_cnt,
                                             u32 segment_id,
                                             u16 block_id,
                                             Bitmask &bitmask) {
    if (bitmask.IsAllTrue()) {
        Search(query, data, dim, dist_f, row_cnt, segment_id, block_id);
        return;
    }
    u32 segment_offset_start = block_id * DEFAULT_BLOCK_CAPACITY;
    for (u64 i = 0; i < this->query_count_; ++i) {
        const QueryElemType *x_i = query + i * dim;
        const QueryElemType *y_j = data;
        for (u16 j = 0; j < row_cnt; ++j, y_j += dim) {
            if (bitmask.IsTrue(j)) {
                if (i == 0) {
                    ++this->total_count_;
                }
                auto dist = dist_f(x_i, y_j, dim);
                result_handler_->AddResult(i, dist, RowID(segment_id, segment_offset_start + j));
            }
        }
    }
}

template <typename QueryElemType, template <typename, typename> typename C, typename DistType>
void MergeKnn<QueryElemType, C, DistType>::Search(const DistType *dist, const RowID *row_ids, u16 count) {
    this->total_count_ += count;
    for (u64 i = 0; i < this->query_count_; ++i) {
        const DistType *d = dist + i * topk_;
        const RowID *r = row_ids + i * topk_;
        for (u16 j = 0; j < count; j++) {
            result_handler_->AddResult(i, d[j], r[j]);
        }
    }
}

template <typename QueryElemType, template <typename, typename> typename C, typename DistType>
void MergeKnn<QueryElemType, C, DistType>::Search(SizeT query_id, const DistType *dist, const RowID *row_ids, u16 count) {
    if (query_id == 0) {
        this->total_count_ += count;
    }
    for (u16 j = 0; j < count; j++) {
        result_handler_->AddResult(query_id, dist[j], row_ids[j]);
    }
}

template <typename QueryElemType, template <typename, typename> typename C, typename DistType>
void MergeKnn<QueryElemType, C, DistType>::Begin() {
    if (this->begin_ || this->query_count_ == 0) {
        return;
    }
    result_handler_->Begin();
    this->begin_ = true;
}

template <typename QueryElemType, template <typename, typename> typename C, typename DistType>
void MergeKnn<QueryElemType, C, DistType>::End() {
    if (!this->begin_)
        return;

    result_handler_->End();

    this->begin_ = false;
}

template <typename QueryElemType, template <typename, typename> typename C, typename DistType>
void MergeKnn<QueryElemType, C, DistType>::EndWithoutSort() {
    if (!this->begin_)
        return;

    result_handler_->EndWithoutSort();

    this->begin_ = false;
}

template <typename QueryElemType, template <typename, typename> typename C, typename DistType>
DistType *MergeKnn<QueryElemType, C, DistType>::GetDistances() const {
    return distance_array_.get();
}

template <typename QueryElemType, template <typename, typename> typename C, typename DistType>
RowID *MergeKnn<QueryElemType, C, DistType>::GetIDs() const {
    return idx_array_.get();
}

template <typename QueryElemType, template <typename, typename> typename C, typename DistType>
DistType *MergeKnn<QueryElemType, C, DistType>::GetDistancesByIdx(u64 idx) const {
    if (idx >= this->query_count_) {
        UnrecoverableError("Query index exceeds the limit");
    }
    return distance_array_.get() + idx * this->topk_;
}

template <typename QueryElemType, template <typename, typename> typename C, typename DistType>
RowID *MergeKnn<QueryElemType, C, DistType>::GetIDsByIdx(u64 idx) const {
    if (idx >= this->query_count_) {
        UnrecoverableError("Query index exceeds the limit");
    }
    return idx_array_.get() + idx * this->topk_;
}

} // namespace infinity