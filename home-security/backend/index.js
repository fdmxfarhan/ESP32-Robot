const express = require('express')
const path = require('path')

const app = express()
const PORT = 3000

// ========================
// CAMERA CONFIGURATION
// ========================
const cameras = [
  {
    name: "Front Door",
    stream: "http://192.168.8.129/stream",
    base: "http://192.168.8.129"
  },
  {
    name: "Street View",
    stream: "http://192.168.8.130/stream",
    base: "http://192.168.8.130"
  }
]

// ========================
// EXPRESS CONFIG
// ========================
app.set('view engine', 'pug')
app.set('views', path.join(__dirname, 'views'))

app.use(express.static(path.join(__dirname, 'public')))

// ========================
// ROUTES
// ========================
app.get('/', (req, res) => {
  res.render('index', { cameras })
})

app.listen(PORT, () => {
  console.log(`Security server running at http://localhost:${PORT}`)
})