if (TARGET ch_contrib::rocksdb)
    set(USE_ROCKSDB 1)
endif()
if (TARGET ch_contrib::bzip2)
    set(USE_BZIP2 1)
endif()
if (TARGET ch_contrib::minizip)
    set(USE_MINIZIP 1)
endif()
if (TARGET ch_contrib::snappy)
    set(USE_SNAPPY 1)
endif()
if (TARGET ch_contrib::brotli)
    set(USE_BROTLI 1)
endif()
if (TARGET ch_contrib::hivemetastore)
    set(USE_HIVE 1)
endif()
if (TARGET ch_contrib::rdkafka)
    set(USE_RDKAFKA 1)
endif()
if (TARGET ch_rust::skim)
    set(USE_SKIM 1)
endif()
if (TARGET ch_rust::prql)
    set(USE_PRQL 1)
endif()
if (TARGET OpenSSL::SSL)
    set(USE_SSL 1)
endif()
if (TARGET ch_contrib::ldap)
    set(USE_LDAP 1)
endif()
if (TARGET ch_contrib::grpc)
    set(USE_GRPC 1)
endif()
if (TARGET ch_contrib::hdfs)
    set(USE_HDFS 1)
endif()
if (TARGET ch_contrib::nuraft)
    set(USE_NURAFT 1)
endif()
if (TARGET ch_contrib::icu)
    set(USE_ICU 1)
endif()
if (TARGET ch_contrib::simdjson)
    set(USE_SIMDJSON 1)
endif()
if (TARGET ch_contrib::rapidjson)
    set(USE_RAPIDJSON 1)
endif()
if (TARGET ch_contrib::azure_sdk)
    set(USE_AZURE_BLOB_STORAGE 1)
endif()
if (TARGET ch_contrib::amqp_cpp)
    set(USE_AMQPCPP 1)
endif()
if (TARGET ch_contrib::nats_io)
    set(USE_NATSIO 1)
endif()
if (TARGET ch_contrib::cassandra)
    set(USE_CASSANDRA 1)
endif()
if (TARGET ch_contrib::base64)
    set(USE_BASE64 1)
endif()
if (TARGET ch_contrib::yaml_cpp)
    set(USE_YAML_CPP 1)
endif()
if (OS_LINUX)
    set(USE_FILELOG 1)
endif()
if (TARGET ch_contrib::sqlite)
    set(USE_SQLITE 1)
endif()
if (TARGET ch_contrib::libpqxx)
    set(USE_LIBPQXX 1)
endif()
if (TARGET ch_contrib::krb5)
    set(USE_KRB5 1)
endif()
if (TARGET ch_contrib::sentry)
    set(USE_SENTRY 1)
endif()
if (TARGET ch_contrib::datasketches)
    set(USE_DATASKETCHES 1)
endif()
if (TARGET ch_contrib::aws_s3)
    set(USE_AWS_S3 1)
endif()
if (TARGET ch_contrib::google_cloud_cpp)
    set(USE_GOOGLE_CLOUD 1)
endif()
if (TARGET ch_contrib::mariadbclient) # ch::mysqlxx
    set(USE_MYSQL 1)
endif()
if (ENABLE_NLP)
    set(USE_NLP 1)
endif()
if (TARGET ch_contrib::ulid)
    set(USE_ULID 1)
endif()
if (TARGET ch_contrib::llvm)
    set(USE_EMBEDDED_COMPILER ${ENABLE_EMBEDDED_COMPILER})
    set(USE_DWARF_PARSER ${ENABLE_DWARF_PARSER})
    set(USE_BLAKE3 ${ENABLE_LIBRARIES})
endif()
if (TARGET ch_contrib::replxx)
    set(USE_REPLXX 1)
endif()
if (TARGET ch_contrib::cpuid)
    set(USE_CPUID 1)
endif()
if (TARGET ch_contrib::jemalloc)
    set(USE_JEMALLOC 1)
endif()
if (TARGET ch_contrib::gwp_asan)
    set(USE_GWP_ASAN 1)
endif()
if (TARGET ch_contrib::h3)
    set(USE_H3 1)
endif()
if (TARGET ch_contrib::s2)
    set(USE_S2_GEOMETRY 1)
endif()
if (TARGET ch_contrib::fastops)
    set(USE_FASTOPS 1)
endif()
if (TARGET ch_contrib::sqids)
    set(USE_SQIDS 1)
endif()
if (TARGET ch_contrib::idna)
    set(USE_IDNA 1)
endif()
if (TARGET ch_contrib::vectorscan)
    set(USE_VECTORSCAN 1)
endif()
if (TARGET ch_contrib::qpl)
    set(USE_QPL 1)
endif()
if (TARGET ch_contrib::qatlib)
    set(USE_QATLIB 1)
endif()
if (TARGET ch_contrib::avrocpp)
    set(USE_AVRO 1)
endif()
if (TARGET ch_contrib::parquet)
    set(USE_PARQUET 1)
    set(USE_ARROW 1)
    set(USE_ORC 1)
endif()
if (TARGET ch_contrib::liburing)
    set(USE_LIBURING 1)
endif ()
if (TARGET ch_contrib::protobuf)
    set(USE_PROTOBUF 1)
endif()
if (TARGET ch_contrib::msgpack)
    set(USE_MSGPACK 1)
endif()
if (TARGET ch_contrib::capnp)
    set(USE_CAPNP 1)
endif()
if (TARGET ch_contrib::bcrypt)
    set(USE_BCRYPT 1)
endif()
if (TARGET ch_contrib::usearch)
    set(USE_USEARCH 1)
endif()
if (TARGET ch_contrib::ssh)
    set(USE_SSH 1)
endif()
if (TARGET ch_contrib::libfiu)
    set(USE_LIBFIU 1)
endif()
if (TARGET ch_contrib::libarchive)
    set(USE_LIBARCHIVE 1)
endif()
if (TARGET ch_contrib::pocketfft)
    set(USE_POCKETFFT 1)
endif()
if (TARGET ch_contrib::prometheus_protobufs)
    set(USE_PROMETHEUS_PROTOBUFS 1)
endif()
if (TARGET ch_contrib::mongocxx)
    set(USE_MONGODB 1)
endif()
if (TARGET ch_contrib::numactl)
    set(USE_NUMACTL 1)
endif()
if (TARGET ch_contrib::delta_kernel_rs)
    set(USE_DELTA_KERNEL_RS 1)
endif()
if (ENABLE_FUZZING)
    set (USE_FUZZING_MODE 1)
endif()
if (ENABLE_BUZZHOUSE)
    set(USE_BUZZHOUSE 1)
endif()
if (TARGET ch_contrib::sha3iuf)
    set(USE_SHA3IUF 1)
endif()

set(SOURCE_DIR ${PROJECT_SOURCE_DIR})
