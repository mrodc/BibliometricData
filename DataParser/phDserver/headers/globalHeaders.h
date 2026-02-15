//
// Created by mrod on 6/20/25.
//

#ifndef GLOBALHEADERS_H
#define GLOBALHEADERS_H

#define FULL_LIMITER true
#define LAST_MATCH true
//System Headers
// #include <bits/stdc++.h>
#include <pwd.h>
#include <array>
#include <vector>
#include <iostream>
#include <regex>
#include <string>
#include <cstdio>
#include <algorithm>
// #include <unistd.h>
#include <sys/types.h>
// #include <pwd.h>
// #include <openssl/sha.h>
#include <fstream>
#include <list>
#include <sys/stat.h>

#include <sstream>
#include <iomanip>
#include <string_view>
#include <filesystem>
#include <span>
#include <cstdlib>
#include <functional>
#include <memory>
#include <thread>
#include <chrono>
#include <functional>
#include <locale>


//MongoDB Headers
#include <mongocxx/client.hpp>
#include <mongocxx/exception/exception.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>
#include <mongocxx/options/find.hpp>

#include <mongocxx/exception/operation_exception.hpp>


//Bson Headers
#include <bsoncxx/oid.hpp>
// #include <bsoncxx/json.hpp>

#
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/builder/stream/array.hpp>

#include <bsoncxx/document/value.hpp>
#include <bsoncxx/document/view.hpp>
#include <bsoncxx/json.hpp>
#include <bsoncxx/types.hpp>
#include <bsoncxx/types/bson_value/value.hpp>

#include <bsoncxx/array/element.hpp>
#include <bsoncxx/array/view.hpp>

#include <bsoncxx/stdx/string_view.hpp>
#include <bsoncxx/string/to_string.hpp>



//Boost headers
#include "boost/date_time/gregorian/gregorian_types.hpp"
#include <boost/algorithm/string.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/regex.hpp>
#include "boost/date_time/time.hpp"
#include "boost/date_time/posix_time/posix_time.hpp" //todo remove?
#include "boost/shared_ptr.hpp"
#include "boost/date_time/dst_rules.hpp"
#include "boost/date_time/time_zone_base.hpp"
#include "boost/date_time/special_defs.hpp"
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/bind_executor.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/core/buffers_to_string.hpp>
#include <boost/asio/buffers_iterator.hpp>
#include <boost/algorithm/string/regex.hpp>


// Poco Headers
#include "Poco/Net/HTTPServer.h"
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPRequestHandlerFactory.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/ServerSocket.h"
#include "Poco/Net/WebSocket.h"
#include "Poco/Net/NetException.h"
#include "Poco/Util/ServerApplication.h"
#include "Poco/Util/Option.h"
#include "Poco/Util/OptionSet.h"
#include "Poco/Util/HelpFormatter.h"
#include "Poco/Format.h"

#include "Poco/Timer.h"
#include "Poco/Thread.h"
#include "Poco/Stopwatch.h"

#include "Poco/Runnable.h"
#include "Poco/DateTimeParser.h"
#include "Poco/LocalDateTime.h"
#include "Poco/DateTime.h"
#include "Poco/DateTimeFormatter.h"
#include "Poco/DateTimeFormat.h"

//Eigen
// #include "eigen3/Eigen/Eigen"

//nlohmann
#include<nlohmann/json.hpp>

//Using directives
namespace fs = std::filesystem;

using namespace std::chrono_literals;

using namespace std;
using namespace boost::algorithm;
using namespace boost::gregorian;

using tcp = boost::asio::ip::tcp;               // from <boost/asio/ip/tcp.hpp>
namespace websocket = boost::beast::websocket;  // from <boost/beast/websocket.hpp>

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>

namespace net = boost::asio;            // from <boost/asio.hpp>

using tcp = boost::asio::ip::tcp;               // from <boost/asio/ip/tcp.hpp>
namespace websocket = boost::beast::websocket;  // from <boost/beast/websocket.hpp>

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>

namespace net = boost::asio;            // from <boost/asio.hpp>

using namespace nlohmann;


using bsoncxx::builder::basic::kvp;
using bsoncxx::builder::basic::make_document;
using bsoncxx::builder::basic::make_array;

using bsoncxx::builder::basic::document;


using Poco::Net::ServerSocket;
using Poco::Net::WebSocket;
using Poco::Net::WebSocketException;
using Poco::Net::HTTPRequestHandler;
using Poco::Net::HTTPRequestHandlerFactory;
using Poco::Net::HTTPServer;
using Poco::Net::HTTPServerRequest;
using Poco::Net::HTTPResponse;
using Poco::Net::HTTPServerResponse;
using Poco::Net::HTTPServerParams;
using Poco::Timestamp;
using Poco::ThreadPool;

using Poco::Util::Application;
using Poco::Util::Option;
using Poco::Util::OptionSet;
using Poco::Util::HelpFormatter;

using Poco::DateTimeParser;
using Poco::DateTimeFormatter;
using Poco::DateTimeFormat;
using Poco::LocalDateTime;

using Poco::DateTime;
using Poco::Timespan;

inline bool mongoInUse = false;


#endif //GLOBALHEADERS_H
