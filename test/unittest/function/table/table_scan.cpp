//
// Created by tangdonghai on 23-9-6.
//

#include "base_test.h"
#include "common/column_vector/column_vector.h"
#include "common/types/data_type.h"
#include "common/types/internal_types.h"
#include "common/types/logical_type.h"
#include "function/table/table_scan.h"
#include "main/config.h"
#include "main/infinity.h"
#include "main/session.h"
#include "main/stats/global_resource_usage.h"
#include "scheduler/pipeline.h"
#include "storage/data_block.h"
#include "storage/meta/catalog.h"
#include "storage/meta/entry/segment_entry.h"
#include "storage/meta/entry/column_data_entry.h"
#include "storage/storage.h"
#include <math.h>

class TableScanTest : public BaseTest {
    void
    SetUp() override {
        infinity::GlobalResourceUsage::Init();
        std::shared_ptr<std::string> config_path = nullptr;
        infinity::Infinity::instance().Init(config_path);

        system("rm -rf /tmp/infinity");
    }

    void
    TearDown() override {
        infinity::Infinity::instance().UnInit();
        EXPECT_EQ(infinity::GlobalResourceUsage::GetObjectCount(), 0);
        EXPECT_EQ(infinity::GlobalResourceUsage::GetRawMemoryCount(), 0);
        infinity::GlobalResourceUsage::UnInit();
    }
};

TEST_F(TableScanTest, block_read_test) {
    using namespace infinity;
    auto catalog = MakeUnique<NewCatalog>(MakeShared<String>("/tmp/infinity"));
    RegisterTableScanFunction(catalog);

    Config config;
    config.Init(nullptr);

    Storage storage(&config);
    storage.Init();

    // create dummy query_context
    UniquePtr<Session> session_ptr = MakeUnique<Session>();
    UniquePtr<QueryContext> query_context = MakeUnique<QueryContext>(
            session_ptr.get(),
            &config,
            nullptr,
            &storage
    );

    Vector<SharedPtr<DataType>> column_types;
    column_types.push_back(MakeShared<DataType>(LogicalType::kInteger));
    // column_types.push_back(MakeShared<DataType>(LogicalType::kDouble));
    // column_types.push_back(MakeShared<DataType>(LogicalType::kCircle));

    // mock segment
    auto segment_entry1 = MakeShared<SegmentEntry>(nullptr);
    segment_entry1->segment_id_ = 1;
    segment_entry1->row_capacity_ = 1000;
    segment_entry1->current_row_ = 1000;
    segment_entry1->columns_.push_back(ColumnDataEntry::MakeNewColumnDataEntry(segment_entry1.get(), 0, 1000,
     MakeShared<DataType>(LogicalType::kInteger),storage.buffer_manager()));

    auto segment_entry2 = MakeShared<SegmentEntry>(nullptr);
    segment_entry2->segment_id_ = 10;
    segment_entry2->row_capacity_ = 10000;
    segment_entry2->current_row_ = 8000;
    segment_entry2->columns_.push_back(ColumnDataEntry::MakeNewColumnDataEntry(segment_entry2.get(), 0, 8000,
     MakeShared<DataType>(LogicalType::kInteger),storage.buffer_manager()));

    auto segment_entry3 = MakeShared<SegmentEntry>(nullptr);
    segment_entry3->segment_id_ = 30;
    segment_entry3->row_capacity_ = 10000;
    segment_entry3->current_row_ = 8000;
    segment_entry3->columns_.push_back(ColumnDataEntry::MakeNewColumnDataEntry(segment_entry3.get(), 0, 8000,
     MakeShared<DataType>(LogicalType::kInteger),storage.buffer_manager()));

    auto segment_entry4 = MakeShared<SegmentEntry>(nullptr);
    segment_entry4->segment_id_ = 20;
    segment_entry4->row_capacity_ = 1000;
    segment_entry4->current_row_ = 408;
    segment_entry4->columns_.push_back(ColumnDataEntry::MakeNewColumnDataEntry(segment_entry4.get(), 0, 1000,
     MakeShared<DataType>(LogicalType::kInteger),storage.buffer_manager()));



    // total_row smaller than block size
    {
        TableCollectionEntry entry(nullptr, nullptr, {} ,TableCollectionType::kTableEntry, 
                         nullptr, 0, 0);
        entry.segments_[segment_entry1->segment_id_] = segment_entry1;

        SizeT total_row = 0;
        for (auto& [_, segment_entry] : entry.segments_) {
            total_row += segment_entry->current_row_;
        }

        Vector<SizeT> column_ids  {0};
        auto table_scan_func = MakeShared<TableScanFunctionData>(&entry, column_ids);
        int times = 0; 
        while (true) {

            DataBlock output;
            output.Init(column_types, 1024);
            auto func = NewCatalog::GetTableFunctionByName(catalog.get(), "seq_scan");
            func->main_function_(query_context.get(), table_scan_func, output);
            if (output.row_count() == 0) {
                break;
            }
            times += 1;
        }

        EXPECT_EQ(times, ceil(total_row / 1024.0));
    }

    
    // total row larger than block size
    {
        TableCollectionEntry entry(nullptr, nullptr, {} ,TableCollectionType::kTableEntry, 
                         nullptr, 0, 0);
        entry.segments_[segment_entry1->segment_id_] = segment_entry1;
        entry.segments_[segment_entry2->segment_id_] = segment_entry2;
        entry.segments_[segment_entry3->segment_id_] = segment_entry3;

        SizeT total_row = 0;
        for (auto& [_, segment_entry] : entry.segments_) {
            total_row += segment_entry->current_row_;
        }

        Vector<SizeT> column_ids  {0};
        auto table_scan_func = MakeShared<TableScanFunctionData>(&entry, column_ids);
        int times = 0; 
        while (true) {

            DataBlock output;
            output.Init(column_types, 1024);
            auto func = NewCatalog::GetTableFunctionByName(catalog.get(), "seq_scan");
            func->main_function_(query_context.get(), table_scan_func, output);
            if (output.row_count() == 0) {
                break;
            }
            times += 1;
        }
        EXPECT_EQ(times, ceil(total_row / 1024.0) );
    }

    // total row is multipy of block size
    {
        TableCollectionEntry entry(nullptr, nullptr, {} ,TableCollectionType::kTableEntry, 
                         nullptr, 0, 0);
        entry.segments_[segment_entry1->segment_id_] = segment_entry1;
        entry.segments_[segment_entry2->segment_id_] = segment_entry2;
        entry.segments_[segment_entry3->segment_id_] = segment_entry3;
        entry.segments_[segment_entry4->segment_id_] = segment_entry4;

        SizeT total_row = 0;
        for (auto& [_, segment_entry] : entry.segments_) {
            total_row += segment_entry->current_row_;
        }

        Vector<SizeT> column_ids  {0};
        auto table_scan_func = MakeShared<TableScanFunctionData>(&entry, column_ids);
        int times = 0; 
        while (true) {

            DataBlock output;
            output.Init(column_types, 1024);
            auto func = NewCatalog::GetTableFunctionByName(catalog.get(), "seq_scan");
            func->main_function_(query_context.get(), table_scan_func, output);
            if (output.row_count() == 0) {
                break;
            }
            times += 1;
        }
        EXPECT_EQ(times, ceil(total_row / 1024.0) );
    }
    
}