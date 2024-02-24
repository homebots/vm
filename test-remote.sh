curl -sS -X POST $ESP_LIVE_URL/prepare
echo Upload
ls firmware/

for i in $(ls -1 firmware | grep '0x'); do
  curl -sS $ESP_LIVE_URL/upload/$i --data-binary @firmware/$i;
done

echo Flash
curl -sS -X POST $ESP_LIVE_URL/flash

echo Reset
curl -sS -X POST $ESP_LIVE_URL/reset
sleep 5

echo Reconnect
curl -sS -X POST $ESP_LIVE_URL/connect

echo Done.