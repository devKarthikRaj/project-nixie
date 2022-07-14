/**
 * 
 * InfluxDBClient.cpp: InfluxDB Client for Arduino
 * 
 * MIT License
 * 
 * Copyright (c) 2020 InfluxData
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
*/
#include "InfluxDbClient.h"
#include <core_version.h>

#define STRHELPER(x) #x
#define STR(x) STRHELPER(x) // stringifier

#if defined(ESP8266)
# define INFLUXDB_CLIENT_PLATFORM "ESP8266"
# define INFLUXDB_CLIENT_PLATFORM_VERSION  STR(ARDUINO_ESP8266_GIT_DESC)
#elif defined(ESP32)
# define INFLUXDB_CLIENT_PLATFORM "ESP32"
# define INFLUXDB_CLIENT_PLATFORM_VERSION  STR(ARDUINO_ESP32_GIT_DESC)
#endif

static const char UserAgent[] PROGMEM = "influxdb-client-arduino/" INFLUXDB_CLIENT_VERSION " (" INFLUXDB_CLIENT_PLATFORM " " INFLUXDB_CLIENT_PLATFORM_VERSION ")";

// Uncomment bellow in case of a problem and rebuild sketch
//#define INFLUXDB_CLIENT_DEBUG_ENABLE
#include "util/debug.h"

static const char UninitializedMessage[] PROGMEM = "Unconfigured instance"; 
static const char TooEarlyMessage[] PROGMEM = "Cannot send request yet because of applied retry strategy. Remaining ";
// This cannot be put to PROGMEM due to the way how it is used
static const char RetryAfter[] = "Retry-After";
static const char TransferEncoding[] = "Transfer-Encoding";

static String escapeJSONString(String &value);
#if defined(ESP8266)  
bool checkMFLN(BearSSL::WiFiClientSecure *client, String url);
#endif
static String precisionToString(WritePrecision precision, uint8_t version = 2) {
    switch(precision) {
        case WritePrecision::US:
            return version==1?"u":"us";
        case WritePrecision::MS:
            return "ms";
        case WritePrecision::NS:
            return "ns";
        case WritePrecision::S:
            return "s";
        default:
            return "";
    }
}

InfluxDBClient::InfluxDBClient() { 
   resetBuffer();
}

InfluxDBClient::InfluxDBClient(const char *serverUrl, const char *db):InfluxDBClient() {
    setConnectionParamsV1(serverUrl, db);
}

InfluxDBClient::InfluxDBClient(const char *serverUrl, const char *org, const char *bucket, const char *authToken):InfluxDBClient(serverUrl, org, bucket, authToken, nullptr) { 
}

InfluxDBClient::InfluxDBClient(const char *serverUrl, const char *org, const char *bucket, const char *authToken, const char *serverCert):InfluxDBClient() {
    setConnectionParams(serverUrl, org, bucket, authToken, serverCert);
}

void InfluxDBClient::setInsecure(bool value){
  _insecure = value;
}

void InfluxDBClient::setConnectionParams(const char *serverUrl, const char *org, const char *bucket, const char *authToken, const char *certInfo) {
    clean();
    _serverUrl = serverUrl;
    _bucket = bucket;
    _org = org;
    _authToken = authToken;
    _certInfo = certInfo;
    _dbVersion = 2;
}

void InfluxDBClient::setConnectionParamsV1(const char *serverUrl, const char *db, const char *user, const char *password, const char *certInfo) {
    clean();
    _serverUrl = serverUrl;
    _bucket = db;
    _user = user;
    _password = password;
    _certInfo = certInfo;
    _dbVersion = 1;
}

bool InfluxDBClient::init() {
    INFLUXDB_CLIENT_DEBUG("[D] Init\n");
    INFLUXDB_CLIENT_DEBUG("[D]  Library version: " INFLUXDB_CLIENT_VERSION "\n");
    INFLUXDB_CLIENT_DEBUG("[D]  Device : " INFLUXDB_CLIENT_PLATFORM "\n");
    INFLUXDB_CLIENT_DEBUG("[D]  SDK version: " INFLUXDB_CLIENT_PLATFORM_VERSION "\n");
    INFLUXDB_CLIENT_DEBUG("[D]  Server url: %s\n", _serverUrl.c_str());
    INFLUXDB_CLIENT_DEBUG("[D]  Org: %s\n", _org.c_str());
    INFLUXDB_CLIENT_DEBUG("[D]  Bucket: %s\n", _bucket.c_str());
    INFLUXDB_CLIENT_DEBUG("[D]  Token: %s\n", _authToken.c_str());
    INFLUXDB_CLIENT_DEBUG("[D]  DB version: %d\n", _dbVersion);
    INFLUXDB_CLIENT_DEBUG("[D]  Connection reuse: %s\n", _httpOptions._connectionReuse?"true":"false");
    if(_serverUrl.length() == 0 || (_dbVersion == 2 && (_org.length() == 0 || _bucket.length() == 0 || _authToken.length() == 0))) {
         INFLUXDB_CLIENT_DEBUG("[E] Invalid parameters\n");
        return false;
    }
    if(_serverUrl.endsWith("/")) {
        _serverUrl = _serverUrl.substring(0,_serverUrl.length()-1);
    }
    setUrls();
    bool https = _serverUrl.startsWith("https");
    if(https) {
#if defined(ESP8266)         
        BearSSL::WiFiClientSecure *wifiClientSec = new BearSSL::WiFiClientSecure;
        if (_insecure) {
            wifiClientSec->setInsecure();
        } else if(_certInfo && strlen_P(_certInfo) > 0) {
            if(strlen_P(_certInfo) > 60 ) { //differentiate fingerprint and cert
                _cert = new BearSSL::X509List(_certInfo); 
                wifiClientSec->setTrustAnchors(_cert);
            } else {
                wifiClientSec->setFingerprint(_certInfo);
            }
        }
        checkMFLN(wifiClientSec, _serverUrl);
#elif defined(ESP32)
        WiFiClientSecure *wifiClientSec = new WiFiClientSecure;  
        if (_insecure) {
#ifndef ARDUINO_ESP32_RELEASE_1_0_4
            // This works only in ESP32 SDK 1.0.5 and higher
            wifiClientSec->setInsecure();
#endif            
        } else if(_certInfo && strlen_P(_certInfo) > 0) { 
           wifiClientSec->setCACert(_certInfo);
        }
#endif    
        _wifiClient = wifiClientSec;
    } else {
        _wifiClient = new WiFiClient;
    }
    if(!_httpClient) {
        _httpClient = new HTTPClient;
    }
    _httpClient->setReuse(_httpOptions._connectionReuse);

    _httpClient->setUserAgent(FPSTR(UserAgent));
    return true;
}

// parse URL for host and port and call probeMaxFragmentLength
#if defined(ESP8266)         
bool checkMFLN(BearSSL::WiFiClientSecure  *client, String url) {
    int index = url.indexOf(':');
     if(index < 0) {
        return false;
    }
    String protocol = url.substring(0, index);
    int port = -1;
    url.remove(0, (index + 3)); // remove http:// or https://

    if (protocol == "http") {
        // set default port for 'http'
        port = 80;
    } else if (protocol == "https") {
        // set default port for 'https'
        port = 443;
    } else {
        return false;
    }
    index = url.indexOf('/');
    String host = url.substring(0, index);
    url.remove(0, index); // remove host 
    // check Authorization
    index = host.indexOf('@');
    if(index >= 0) {
        host.remove(0, index + 1); // remove auth part including @
    }
    // get port
    index = host.indexOf(':');
    if(index >= 0) {
        String portS = host;
        host = host.substring(0, index); // hostname
        portS.remove(0, (index + 1)); // remove hostname + :
        port = portS.toInt(); // get port
    }
    INFLUXDB_CLIENT_DEBUG("[D] probeMaxFragmentLength to %s:%d\n", host.c_str(), port);
    bool mfln = client->probeMaxFragmentLength(host, port, 1024);
    INFLUXDB_CLIENT_DEBUG("[D]  MFLN:%s\n", mfln ? "yes" : "no");
    if (mfln) {
        client->setBufferSizes(1024, 1024);
    } 
    return mfln;
}
#endif //ESP8266

InfluxDBClient::~InfluxDBClient() {
     if(_writeBuffer) {
        delete [] _writeBuffer;
        _writeBuffer = nullptr;
        _bufferPointer = 0;
        _batchPointer = 0;
        _bufferCeiling = 0;
    }
    clean();
}

void InfluxDBClient::clean() {
    if(_httpClient) {
        delete _httpClient;
        _httpClient = nullptr;
    }
    if(_wifiClient) {
        delete _wifiClient;
        _wifiClient = nullptr;
    }
#if defined(ESP8266)     
    if(_cert) {
        delete _cert;
        _cert = nullptr;
    }
#endif
    _lastStatusCode = 0;
    _lastErrorResponse = "";
    _lastFlushed = 0;
    _lastRequestTime = 0;
    _lastRetryAfter = 0;
}

void InfluxDBClient::setUrls() {
    INFLUXDB_CLIENT_DEBUG("[D] setUrls\n");
    if(_dbVersion == 2) {
        _writeUrl = _serverUrl;
        _writeUrl += "/api/v2/write?org=";
        _writeUrl +=  urlEncode(_org.c_str());
        _writeUrl += "&bucket=";
        _writeUrl += urlEncode(_bucket.c_str());
        INFLUXDB_CLIENT_DEBUG("[D]  writeUrl: %s\n", _writeUrl.c_str());
        _queryUrl = _serverUrl;
        _queryUrl += "/api/v2/query?org=";
        _queryUrl +=  urlEncode(_org.c_str());
        INFLUXDB_CLIENT_DEBUG("[D]  queryUrl: %s\n", _queryUrl.c_str());
    } else {
        _writeUrl = _serverUrl;
        _writeUrl += "/write?db=";
        _writeUrl += urlEncode(_bucket.c_str());
        _queryUrl = _serverUrl;
        _queryUrl += "/api/v2/query";
        if(_user.length() > 0 && _password.length() > 0) {
            String auth = "&u=";
            auth += urlEncode(_user.c_str());
            auth += "&p=";
            auth += urlEncode(_password.c_str());
            _writeUrl += auth;  
            _queryUrl += "?";
            _queryUrl += auth;
        }
        INFLUXDB_CLIENT_DEBUG("[D]  writeUrl: %s\n", _writeUrl.c_str());
        INFLUXDB_CLIENT_DEBUG("[D]  queryUrl: %s\n", _queryUrl.c_str());
    }
    if(_writeOptions._writePrecision != WritePrecision::NoTime) {
        _writeUrl += "&precision=";
        _writeUrl += precisionToString(_writeOptions._writePrecision, _dbVersion);
        INFLUXDB_CLIENT_DEBUG("[D]  writeUrl: %s\n", _writeUrl.c_str());
    }
    
}

void InfluxDBClient::setWriteOptions(WritePrecision precision, uint16_t batchSize, uint16_t bufferSize, uint16_t flushInterval, bool preserveConnection) {
    setWriteOptions(WriteOptions().writePrecision(precision).batchSize(batchSize).bufferSize(bufferSize).flushInterval(flushInterval));
    setHTTPOptions(_httpOptions.connectionReuse(preserveConnection));
}

void InfluxDBClient::setWriteOptions(const WriteOptions & writeOptions) {
    if(_writeOptions._writePrecision != writeOptions._writePrecision) {
        _writeOptions._writePrecision = writeOptions._writePrecision;
        setUrls();
    }
    if(writeOptions._batchSize > 0) {
        _writeOptions._batchSize = writeOptions._batchSize;
    }
    if(_writeOptions._bufferSize > 0 && writeOptions._bufferSize > 0 && _writeOptions._bufferSize != writeOptions._bufferSize) {
        _writeOptions._bufferSize = writeOptions._bufferSize;
        if(_writeOptions._bufferSize <  2*_writeOptions._batchSize) {
            _writeOptions._bufferSize = 2*_writeOptions._batchSize;
            INFLUXDB_CLIENT_DEBUG("[D] Changing buffer size to %d\n", _writeOptions._bufferSize);
        }
        resetBuffer();
    }
    _writeOptions._flushInterval = writeOptions._flushInterval;
    _writeOptions._retryInterval = writeOptions._retryInterval;
    _writeOptions._maxRetryInterval = writeOptions._maxRetryInterval;
    _writeOptions._maxRetryAttempts = writeOptions._maxRetryAttempts;
    _writeOptions._defaultTags = writeOptions._defaultTags;
}

void InfluxDBClient::setHTTPOptions(const HTTPOptions & httpOptions) {
    _httpOptions = httpOptions;
    if(!_httpClient) {
        _httpClient = new HTTPClient;
    }
    _httpClient->setReuse(_httpOptions._connectionReuse);
    _httpClient->setTimeout(_httpOptions._httpReadTimeout);
}

void InfluxDBClient::resetBuffer() {
    if(_writeBuffer) {
        for(int i=0;i<_writeBufferSize;i++) {
            delete _writeBuffer[i];
        }
        delete [] _writeBuffer;
    }
    _writeBufferSize = _writeOptions._bufferSize/_writeOptions._batchSize;
    if(_writeBufferSize < 2) {
        _writeBufferSize = 2;
    }
    INFLUXDB_CLIENT_DEBUG("[D] Reset buffer: writeBuffSize: %d\n", _writeBufferSize);
    _writeBuffer = new Batch*[_writeBufferSize];
    for(int i=0;i<_writeBufferSize;i++) {
        _writeBuffer[i] = nullptr;
    }
    _bufferPointer = 0;
    _batchPointer = 0;
    _bufferCeiling = 0;
}

void InfluxDBClient::reserveBuffer(int size) {
    if(size > _writeBufferSize) {
        Batch **newBuffer = new Batch*[size];
        INFLUXDB_CLIENT_DEBUG("[D] Resizing buffer from %d to %d\n",_writeBufferSize, size);
        for(int i=0;i<_bufferCeiling; i++) {
            newBuffer[i] = _writeBuffer[i];
        }
        
        delete [] _writeBuffer;
        _writeBuffer = newBuffer;
        _writeBufferSize = size;
    }
}

bool InfluxDBClient::writePoint(Point & point) {
    if (point.hasFields()) {
        if(_writeOptions._writePrecision != WritePrecision::NoTime && !point.hasTime()) {
            point.setTime(_writeOptions._writePrecision);
        }
        String line = pointToLineProtocol(point);
        return writeRecord(line);
    }
    return false;
}

bool InfluxDBClient::Batch::append(String &line) {
    if(pointer == _size) {
        //overwriting, clean buffer
        for(int i=0;i< _size; i++) {
            buffer[i] = (const char *)nullptr; 
        }
        pointer = 0;
    } 
    buffer[pointer] = line;
    ++pointer;
    return isFull();
}

char * InfluxDBClient::Batch::createData() {
     int length = 0; 
     char *buff = nullptr;
     for(int c=0; c < pointer; c++) {
        length += buffer[c].length();
        yield();
    }
    //create buffer for all lines including new line char and terminating char
    if(length) {
        buff = new char[length + pointer + 1];
        if(buff) {
            buff[0] = 0;
            for(int c=0; c < pointer; c++) {
                strcat(buff+strlen(buff), buffer[c].c_str());
                strcat(buff+strlen(buff), "\n");
                yield();
            }
        }
    }
    return buff;
}

bool InfluxDBClient::writeRecord(String &record) {
    if(!_writeBuffer[_bufferPointer]) {
        _writeBuffer[_bufferPointer] = new Batch(_writeOptions._batchSize);
    }
    if(isBufferFull() && _batchPointer <= _bufferPointer) {
        // When we are overwriting buffer and nothing is written, batchPointer must point to the oldest point
        _batchPointer = _bufferPointer+1;
        if(_batchPointer == _writeBufferSize) {
            _batchPointer = 0;
        }
    }
    if(_writeBuffer[_bufferPointer]->append(record)) { //we reached batch size
        _bufferPointer++;
        if(_bufferPointer == _writeBufferSize) { // writeBuffer is full
            _bufferPointer = 0;
            INFLUXDB_CLIENT_DEBUG("[W] Reached write buffer size, old points will be overwritten\n");
        } 

        if(_bufferCeiling < _writeBufferSize) {
            _bufferCeiling++;
        }
    } 
    INFLUXDB_CLIENT_DEBUG("[D] writeRecord: bufferPointer: %d, batchPointer: %d, _bufferCeiling: %d\n", _bufferPointer, _batchPointer, _bufferCeiling);    
    return checkBuffer();
}

bool InfluxDBClient::checkBuffer() {
    // in case we (over)reach batchSize with non full buffer
    bool bufferReachedBatchsize = _writeBuffer[_batchPointer] && _writeBuffer[_batchPointer]->isFull();
    // or flush interval timed out
    bool flushTimeout = _writeOptions._flushInterval > 0 && _lastFlushed > 0 && (millis()/1000 - _lastFlushed) > _writeOptions._flushInterval; 

    if(bufferReachedBatchsize || flushTimeout || isBufferFull() ) {
        INFLUXDB_CLIENT_DEBUG("[D] Flushing buffer: is oversized %s, is timeout %s, is buffer full %s\n", bufferReachedBatchsize?"true":"false",flushTimeout?"true":"false", isBufferFull()?"true":"false");
       return flushBufferInternal(true);
    } 
    return true;
}

bool InfluxDBClient::flushBuffer() {
    return flushBufferInternal(false);
}

uint32_t InfluxDBClient::getRemainingRetryTime() {
    uint32_t rem = 0;
    if(_lastRetryAfter > 0) {
        int32_t diff = _lastRetryAfter - (millis()-_lastRequestTime)/1000;
        rem  =  diff<0?0:(uint32_t)diff;
    }
    return rem;
}

bool InfluxDBClient::flushBufferInternal(bool flashOnlyFull) {
    uint32_t rwt = getRemainingRetryTime();
    if(rwt > 0) {
        INFLUXDB_CLIENT_DEBUG("[W] Cannot write yet, pause %ds, %ds yet\n", _lastRetryAfter, rwt);
        // retry after period didn't run out yet
        _lastStatusCode = 0;
        _lastErrorResponse = FPSTR(TooEarlyMessage);
        _lastErrorResponse += String(rwt);
        _lastErrorResponse += "s";
        return false;
    }
    char *data;
    bool success = true;
    // send all batches, It could happen there was long network outage and buffer is full
    while(_writeBuffer[_batchPointer] && (!flashOnlyFull ||  _writeBuffer[_batchPointer]->isFull())) {
        data = _writeBuffer[_batchPointer]->createData();
        if(!_writeBuffer[_batchPointer]->isFull() && _writeBuffer[_batchPointer]->retryCount == 0 ) { //do not increase pointer in case of retrying
            // points will be written so increase _bufferPointer as it happen when buffer is flushed when is full
            if(++_bufferPointer == _writeBufferSize) {
                _bufferPointer = 0;
            }
        }

        INFLUXDB_CLIENT_DEBUG("[D] Writing batch, batchpointer: %d, size %d\n", _batchPointer, _writeBuffer[_batchPointer]->pointer);
        if(data) {
            int statusCode = postData(data);
            delete [] data;
            // retry on unsuccessfull connection or retryable status codes
            bool retry = statusCode < 0 || statusCode >= 429;
            success = statusCode >= 200 && statusCode < 300;
            // advance even on message failure x e <300;429)
            if(success || !retry) {
                _lastFlushed = millis()/1000;
                dropCurrentBatch();
            } else if(retry) {
                _writeBuffer[_batchPointer]->retryCount++;
                if(statusCode > 0) { //apply retry strategy only in case of HTTP errors
                    if(_writeBuffer[_batchPointer]->retryCount > _writeOptions._maxRetryAttempts) {
                        INFLUXDB_CLIENT_DEBUG("[D] Reached max retry count, dropping batch\n");
                        dropCurrentBatch();
                    }
                    if(!_lastRetryAfter) {
                        _lastRetryAfter = _writeOptions._retryInterval;
                        if(_writeBuffer[_batchPointer]) {
                            for(int i=1;i<_writeBuffer[_batchPointer]->retryCount;i++) {
                                _lastRetryAfter *= _writeOptions._retryInterval;
                            }
                            if(_lastRetryAfter > _writeOptions._maxRetryInterval) {
                                _lastRetryAfter = _writeOptions._maxRetryInterval;
                            }
                        }
                    }
                } 
                INFLUXDB_CLIENT_DEBUG("[D] Leaving data in buffer for retry, retryInterval: %d\n",_lastRetryAfter);
                // in case of retryable failure break loop
                break;
            }
        }
       yield();
    }
    //Have we emptied the buffer?
    INFLUXDB_CLIENT_DEBUG("[D] Success: %d, _bufferPointer: %d, _batchPointer: %d, _writeBuffer[_bufferPointer]_%p\n",success,_bufferPointer,_batchPointer, _writeBuffer[_bufferPointer]);
    if(_batchPointer == _bufferPointer && !_writeBuffer[_bufferPointer]) {
        _bufferPointer = 0;
        _batchPointer = 0;
        _bufferCeiling = 0;
        INFLUXDB_CLIENT_DEBUG("[D] Buffer empty\n");
    }
    return success;
}

void  InfluxDBClient::dropCurrentBatch() {
    delete _writeBuffer[_batchPointer];
    _writeBuffer[_batchPointer] = nullptr;
    _batchPointer++;
    //did we got over top?
    if(_batchPointer == _writeBufferSize) {
        // restart _batchPointer in ring buffer from start
        _batchPointer = 0;
        // we reached buffer size, that means buffer was full and now lower ceiling 
        _bufferCeiling = _bufferPointer;
    }
    INFLUXDB_CLIENT_DEBUG("[D] Dropped batch, batchpointer: %d\n", _batchPointer);
}

String InfluxDBClient::pointToLineProtocol(const Point& point) {
    return point.createLineProtocol(_writeOptions._defaultTags);
}

bool InfluxDBClient::validateConnection() {
    if(!_wifiClient && !init()) {
        _lastStatusCode = 0;
        _lastErrorResponse = FPSTR(UninitializedMessage);
        return false;
    }
    // on version 1.x /ping will by default return status code 204, without verbose
    String url = _serverUrl + (_dbVersion==2?"/health":"/ping?verbose=true");
    if(_dbVersion==1 && _user.length() > 0 && _password.length() > 0) {
        url += "&u=";
        url += urlEncode(_user.c_str());
        url += "&p=";
        url += urlEncode(_password.c_str());
    }
    INFLUXDB_CLIENT_DEBUG("[D] Validating connection to %s\n", url.c_str());

    if(!_httpClient->begin(*_wifiClient, url)) {
        INFLUXDB_CLIENT_DEBUG("[E] begin failed\n");
        return false;
    }
    _httpClient->addHeader(F("Accept"), F("application/json"));
    
    _lastStatusCode = _httpClient->GET();

   _lastErrorResponse = "";
    
    afterRequest(200, false);

    _httpClient->end();

    return _lastStatusCode == 200;
}

void InfluxDBClient::beforeRequest() {
    if(_authToken.length() > 0) {
        _httpClient->addHeader(F("Authorization"), "Token " + _authToken);
    }
    const char * headerKeys[] = {RetryAfter, TransferEncoding} ;
    _httpClient->collectHeaders(headerKeys, 2);
}

int InfluxDBClient::postData(const char *data) {
    if(!_wifiClient && !init()) {
        _lastStatusCode = 0;
        _lastErrorResponse = FPSTR(UninitializedMessage);
        return 0;
    }
    if(data) {
        INFLUXDB_CLIENT_DEBUG("[D] Writing to %s\n", _writeUrl.c_str());
        if(!_httpClient->begin(*_wifiClient, _writeUrl)) {
            INFLUXDB_CLIENT_DEBUG("[E] Begin failed\n");
            return false;
        }
        INFLUXDB_CLIENT_DEBUG("[D] Sending:\n%s\n", data);       

        _httpClient->addHeader(F("Content-Type"), F("text/plain"));   
        
        beforeRequest();        
        
        _lastStatusCode = _httpClient->POST((uint8_t*)data, strlen(data));
        
        afterRequest(204);

        
        _httpClient->end();
    } 
    return _lastStatusCode;
}



static const char QueryDialect[] PROGMEM = "\
\"dialect\": {\
\"annotations\": [\
\"datatype\"\
],\
\"dateTimeFormat\": \"RFC3339\",\
\"header\": true,\
\"delimiter\": \",\",\
\"commentPrefix\": \"#\"\
}}";

FluxQueryResult InfluxDBClient::query(String fluxQuery) {
    uint32_t rwt = getRemainingRetryTime();
    if(rwt > 0) {
        INFLUXDB_CLIENT_DEBUG("[W] Cannot query yet, pause %ds, %ds yet\n", _lastRetryAfter, rwt);
        // retry after period didn't run out yet
        String mess = FPSTR(TooEarlyMessage);
        mess += String(rwt);
        mess += "s";
        return FluxQueryResult(mess);
    }
    if(!_wifiClient && !init()) {
        _lastStatusCode = 0;
        _lastErrorResponse = FPSTR(UninitializedMessage);
        return FluxQueryResult(_lastErrorResponse);
    }
    INFLUXDB_CLIENT_DEBUG("[D] Query to %s\n", _queryUrl.c_str());
    if(!_httpClient->begin(*_wifiClient, _queryUrl)) {
        INFLUXDB_CLIENT_DEBUG("[E] begin failed\n");
        return FluxQueryResult("");;
    }
    _httpClient->addHeader(F("Content-Type"), F("application/json"));
    
    beforeRequest();

    INFLUXDB_CLIENT_DEBUG("[D] JSON query:\n%s\n", fluxQuery.c_str());

    String body = F("{\"type\":\"flux\",\"query\":\"");
    body += escapeJSONString(fluxQuery) + "\",";
    body += FPSTR(QueryDialect);

    _lastStatusCode = _httpClient->POST(body);
    
    afterRequest(200);
    if(_lastStatusCode == 200) {
        bool chunked = false;
        if(_httpClient->hasHeader(TransferEncoding)) {
            String header = _httpClient->header(TransferEncoding);
            chunked = header.equalsIgnoreCase("chunked");
        }
        INFLUXDB_CLIENT_DEBUG("[D] chunked: %s\n", chunked?"true":"false");
        HttpStreamScanner *scanner = new HttpStreamScanner(_httpClient, chunked);
        CsvReader *reader = new CsvReader(scanner);
        
        return FluxQueryResult(reader);
    } else {
        _httpClient->end();
        return FluxQueryResult(_lastErrorResponse);
    }
}

void InfluxDBClient::afterRequest(int expectedStatusCode,  bool modifyLastConnStatus) {
    if(modifyLastConnStatus) {
        _lastRequestTime = millis();
        INFLUXDB_CLIENT_DEBUG("[D] HTTP status code - %d\n", _lastStatusCode);
        _lastRetryAfter = 0;
        if(_lastStatusCode >= 429) { //retryable server errors
            if(_httpClient->hasHeader(RetryAfter)) {
                _lastRetryAfter = _httpClient->header(RetryAfter).toInt();
                INFLUXDB_CLIENT_DEBUG("[D] Reply after - %d\n", _lastRetryAfter);
            }
        }
    }
    _lastErrorResponse = "";
    if(_lastStatusCode != expectedStatusCode) {
        if(_lastStatusCode > 0) {
            _lastErrorResponse = _httpClient->getString();
            INFLUXDB_CLIENT_DEBUG("[D] Response:\n%s\n", _lastErrorResponse.c_str());
        } else {
            _lastErrorResponse = _httpClient->errorToString(_lastStatusCode);
            INFLUXDB_CLIENT_DEBUG("[E] Error - %s\n", _lastErrorResponse.c_str());
        }
    }
}

static String escapeJSONString(String &value) {
    String ret;
    int d = 0;
    int i,from = 0;
    while((i = value.indexOf('"',from)) > -1) {
        d++;
        if(i == (int)value.length()-1) {
            break;
        }
        from = i+1;
    }
    ret.reserve(value.length()+d); //most probably we will escape just double quotes
    for (char c: value)
    {
        switch (c)
        {
            case '"': ret += "\\\""; break;
            case '\\': ret += "\\\\"; break;
            case '\b': ret += "\\b"; break;
            case '\f': ret += "\\f"; break;
            case '\n': ret += "\\n"; break;
            case '\r': ret += "\\r"; break;
            case '\t': ret += "\\t"; break;
            default:
                if (c <= '\x1f') {
                    ret += "\\u";
                    char buf[3 + 8 * sizeof(unsigned int)];
                    sprintf(buf,  "\\u%04u", c);
                    ret += buf;
                } else {
                    ret += c;
                }
        }
    }
    return ret;
}
