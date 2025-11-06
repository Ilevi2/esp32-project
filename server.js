const mqtt = require("mqtt");
const express = require("express");
const app = express();
const PORT = 3000;

app.use(express.json());

// MQTT broker (public)
const broker = "mqtt://test.mosquitto.org";
const topicSub = "esp32/dht22/data";
const topicPub = "esp32/led/control";

const client = mqtt.connect(broker);

// === MQTT Connection ===
client.on("connect", () => {
  console.log("✅ Connected to MQTT broker");
  client.subscribe(topicSub, (err) => {
    if (!err) console.log(`📡 Subscribed to topic: ${topicSub}`);
  });
});

// === Receive Sensor Data ===
client.on("message", (topic, message) => {
  if (topic === topicSub) {
    console.log("🌡️ Data received from ESP32:", message.toString());
  }
});

// === Control LED via API ===
app.post("/led", (req, res) => {
  const { state } = req.body; // {"state":"ON"} or {"state":"OFF"}

  if (state !== "ON" && state !== "OFF") {
    return res.status(400).send({ error: "Invalid LED state" });
  }

  client.publish(topicPub, state);
  console.log(`💡 Sent LED command: ${state}`);
  res.send({ message: `LED turned ${state}` });
});

// === Start server ===
app.listen(PORT, () => {
  console.log(`🚀 Middleware running at http://localhost:${PORT}`);
});
