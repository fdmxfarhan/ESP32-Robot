const express = require("express");
const bodyParser = require("body-parser");
const app = express();

const PORT = 3008;

let clients = [];
let lastCommand = "STOP";

// --------------------
// Middleware
// --------------------
app.set("view engine", "ejs");
app.use(express.static("public"));
app.use(bodyParser.json());

// --------------------
// Dashboard Page
// --------------------
app.get("/", (req, res) => {
  res.render("index");
});

// --------------------
// MJPEG Stream
// --------------------
app.get("/stream", (req, res) => {
  res.writeHead(200, {
    "Content-Type": "multipart/x-mixed-replace; boundary=frame",
    "Cache-Control": "no-cache",
    "Connection": "keep-alive",
  });

  clients.push(res);

  req.on("close", () => {
    clients = clients.filter(c => c !== res);
  });
});

// --------------------
// ESP32 uploads frames
// --------------------
app.post("/upload", (req, res) => {
  let buffer = [];

  req.on("data", chunk => buffer.push(chunk));
  req.on("end", () => {
    const frame = Buffer.concat(buffer);

    clients.forEach(client => {
      client.write("--frame\r\n");
      client.write("Content-Type: image/jpeg\r\n");
      client.write(`Content-Length: ${frame.length}\r\n\r\n`);
      client.write(frame);
      client.write("\r\n");
    });

    res.sendStatus(200);
  });
});

// --------------------
// Browser sends command
// --------------------
app.post("/control", (req, res) => {
  lastCommand = req.body.command;
  console.log("Command:", lastCommand);
  res.json({ status: "ok" });
});

// --------------------
// ESP32 fetches command
// --------------------
app.get("/command", (req, res) => {
  res.json({ command: lastCommand });
});

// --------------------
app.listen(PORT, "0.0.0.0", () => {
  console.log(`Server running on 0.0.0.0:${PORT}`);
});
