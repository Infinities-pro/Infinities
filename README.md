<div align="center">
  <img width="700" src="assets/logo2.jpg" alt="Logo 2"/>
</div>


<p align="center">
    <b>The AI-native database built for LLM applications, providing incredibly fast hybrid search of dense embedding, sparse embedding, tensor and full-text</b>
</p>

<h4 align="center">
  <a href="https://docs.infinities.pro">Document</a> |
  <a href="https://docs.infinities.pro/benchmark">Benchmark</a> |
  <a href="https://x.com/infinities_pro">Twitter</a> |
  <a href="https://discord.gg/jEfRUwEYEV">Discord</a>
</h4>


Infinity is a cutting-edge AI-native database that provides a wide range of search capabilities for rich data types such as dense vector, sparse vector, tensor, full-text, and structured data. It provides robust support for various LLM applications, including search, recommenders, question-answering, conversational AI, copilot, content generation, and many more **RAG** (Retrieval-augmented Generation) applications.

- [Key Features](#-key-features)
- [Get Started](#-get-started)
- [Document](#-document)
- [Roadmap](#-roadmap)
- [Community](#-community)

## ⚡️ Performance

<div class="column" align="middle">
  <img src="https://github.com/user-attachments/assets/c4c98e23-62ac-4d1a-82e5-614bca96fe0a" alt="Infinity performance comparison"/>
</div>

## 🌟 Key Features

Infinity comes with high performance, flexibility, ease-of-use, and many features designed to address the challenges facing the next-generation AI applications:

### 🚀 Incredibly fast

- Achieves 0.1 milliseconds query latency and 15K+ QPS on million-scale vector datasets.
- Achieves 1 millisecond latency and 12K+ QPS in full-text search on 33M documents.

> See the [Benchmark report](https://docs.infinities.pro/benchmark) for more information.

### 🔮 Powerful search

- Supports a hybrid search of dense embedding, sparse embedding, tensor, and full text, in addition to filtering.
- Supports several types of rerankers including RRF, weighted sum and **ColBERT**.

### 🍔 Rich data types

Supports a wide range of data types including strings, numerics, vectors, and more.

### 🎁 Ease-of-use

- Intuitive Python API. See the [Python API](https://docs.infinities.pro/pysdk_api_reference)
- A single-binary architecture with no dependencies, making deployment a breeze.
- Embedded in Python as a module and friendly to AI developers.  

## 🎮 Get Started

Infinity supports two working modes, embedded mode and client-server mode. Infinity's embedded mode enables you to quickly embed Infinity into your Python applications, without the need to connect to a separate backend server. The following shows how to operate in embedded mode:

   ```bash
   pip install infinity-embedded-sdk==0.6.0.dev2
   ```
   Use Infinity to conduct a dense vector search:
   ```python
   import infinity_embedded

   # Connect to infinity
   infinity_object = infinity_embedded.connect("/absolute/path/to/save/to")
   # Retrieve a database object named default_db
   db_object = infinity_object.get_database("default_db")
   # Create a table with an integer column, a varchar column, and a dense vector column
   table_object = db_object.create_table("my_table", {"num": {"type": "integer"}, "body": {"type": "varchar"}, "vec": {"type": "vector, 4, float"}})
   # Insert two rows into the table
   table_object.insert([{"num": 1, "body": "unnecessary and harmful", "vec": [1.0, 1.2, 0.8, 0.9]}])
   table_object.insert([{"num": 2, "body": "Office for Harmful Blooms", "vec": [4.0, 4.2, 4.3, 4.5]}])
   # Conduct a dense vector search
   res = table_object.output(["*"])
                     .match_dense("vec", [3.0, 2.8, 2.7, 3.1], "float", "ip", 2)
                     .to_pl()
   print(res)
   ```

> 💡 For more information about Infinity's Python API, see the [Python API Reference](https://docs.infinities.pro/pysdk_api_reference).

#### 🔧 Deploy Infinity in client-server mode

If you wish to deploy Infinity with the server and client as separate processes, see the [Deploy infinity server](https://docs.infinities.pro/deploy_infinity_server) guide.

#### 🔧 Build from Source

See the [Build from Source](https://docs.infinities.pro/build_from_source) guide.

## 📚 Document

- [Quickstart](https://docs.infinities.pro/)
- [Python API](https://docs.infinities.pro/pysdk_api_reference)
- [HTTP API](https://docs.infinities.pro/http_api_reference)
- [References](https://docs.infinities.pro/references)
- [FAQ](https://docs.infinities.pro/FAQ)

## 📜 Roadmap

See the [Infinity Roadmap 2025](https://infinities.pro/roadmap)

## 🙌 Community

- [APP](https://infinities.pro/)
- [Twitter](https://x.com/infinities_pro)

