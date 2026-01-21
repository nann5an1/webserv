#!/usr/bin/env bash

SERVER1="127.0.0.1:8081"
SERVER2="127.0.0.1:8082"

ITER=30
UPLOAD_FILE="curl_upload_test.txt"

echo "curl upload test" > $UPLOAD_FILE

curl_test() {
    METHOD=$1
    URL=$2
    DATA=$3

    echo "================================================"
    echo "METHOD : $METHOD"
    echo "URL    : $URL"
    echo "REPEAT : $ITER"
    echo "------------------------------------------------"

    for i in $(seq 1 $ITER); do
        if [ "$METHOD" = "GET" ]; then
            curl -s \
                 -o /dev/null \
                 -w "[$i] code=%{http_code} time=%{time_total}\n" \
                 "$URL"

        elif [ "$METHOD" = "POST" ]; then
            curl -s \
                 -o /dev/null \
                 -w "[$i] code=%{http_code} time=%{time_total}\n" \
                 -X POST \
                 --data "$DATA" \
                 "$URL"

        elif [ "$METHOD" = "DELETE" ]; then
            curl -s \
                 -o /dev/null \
                 -w "[$i] code=%{http_code} time=%{time_total}\n" \
                 -X DELETE \
                 "$URL"
        fi
    done

    echo
}

###################################
# ROOT /
###################################
curl_test GET    "http://$SERVER1/"
curl_test POST   "http://$SERVER1/" "hello=root"

###################################
# /static
###################################
curl_test GET    "http://$SERVER1/static/"
curl_test DELETE "http://$SERVER1/static/test_delete_file"

###################################
# /redirect
###################################
curl_test GET "http://$SERVER1/redirect"

###################################
# /autoindex
###################################
curl_test GET  "http://$SERVER1/autoindex/"
curl_test POST "http://$SERVER1/autoindex/" "auto=index"

###################################
# /upload
###################################
curl_test GET  "http://$SERVER1/upload/"
curl_test POST "http://$SERVER1/upload/" "@$UPLOAD_FILE"
curl_test DELETE "http://$SERVER1/upload/curl_upload_test.txt"

###################################
# /cgi
###################################
curl_test GET    "http://$SERVER1/cgi"
curl_test DELETE "http://$SERVER1/cgi"

###################################
# REDIRECT TARGET SERVER
###################################
curl_test GET "http://$SERVER2/"

rm -f $UPLOAD_FILE
