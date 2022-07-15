const mqtt = require('mqtt')
const fs = require('fs')
const { Command } = require('commander')

const program = new Command()
program
  .option('-p, --protocol <type>', 'connect protocol: mqtt, mqtts, ws, wss. default is mqtt', 'mqtt')
  .parse(process.argv)

const host = 'ngoinhaiot.com'
const port = '1111'
const clientId = `mqtt_${Math.random().toString(16).slice(3)}`

// connect options
const OPTIONS = {
  clientId,
  clean: true,
  connectTimeout: 4000,
  username: 'hoangtoancsgl',
  password: '850B3436127D4E73',
  reconnectPeriod: 1000,
}
// protocol list
const PROTOCOLS = ['mqtt', 'mqtts', 'ws', 'wss']

// default is mqtt, unencrypted tcp connection
let connectUrl = `mqtt://${host}:${port}`
if (program.protocol && PROTOCOLS.indexOf(program.protocol) === -1) {
  console.log('protocol must one of mqtt, mqtts, ws, wss.')
} else if (program.protocol === 'mqtts') {
  // mqtts， encrypted tcp connection
  connectUrl = `mqtts://${host}:8883`
  OPTIONS['ca'] = fs.readFileSync('./ngoinhaiot.com-ca.crt')
} else if (program.protocol === 'ws') {
  // ws, unencrypted WebSocket connection
  const mountPath = '/mqtt' // mount path, connect emqx via WebSocket
  connectUrl = `ws://${host}:8083${mountPath}`
} else if (program.protocol === 'wss') {
  // wss, encrypted WebSocket connection
  const mountPath = '/mqtt' // mount path, connect emqx via WebSocket
  connectUrl = `wss://${host}:8084${mountPath}`
  OPTIONS['ca'] = fs.readFileSync('./ngoinhaiot.com-ca.crt')
} else {}

const topic = '#'

const client = mqtt.connect(connectUrl, OPTIONS)

client.on('connect', () => {
  console.log(`${program.protocol}: Connected`)
  client.subscribe([topic], () => {
    console.log(`${program.protocol}: Subscribe to topic '${topic}'`)
  })
  client.publish("hoangtoancsgl/test_mqtt_node_js", 'ok', { qos: 0, retain: false }, (error) => {
    if (error) {
      console.error(error)
    }
  })
})

client.on('reconnect', (error) => {
  console.log(`Reconnecting(${program.protocol}):`, error)
})

client.on('error', (error) => {
  console.log(`Cannot connect(${program.protocol}):`, error)
})

client.on('message', (topic, payload) => {
  console.log('Received Message:', topic, payload.toString())
})