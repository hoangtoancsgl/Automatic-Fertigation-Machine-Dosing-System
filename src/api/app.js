require("dotenv").config();
const express = require("express");
const mongoose = require("mongoose");
const cors = require("cors");
const mqtt = require("mqtt");
const fs = require("fs");
const { Command } = require("commander");
const program = new Command();
const { MongoClient, ObjectId } = require("mongodb");

const authRoutes = require("./routes/authRoutes");
const dataRoutes = require("./routes/dataRoutes");
const totalvolumeRoutes = require("./routes/totalVolumeRoutes");
const changeNutriRoutes = require("./routes/changeNutriRoutes");
const setVolumeRoutes = require("./routes/setVolumeRoutes");
const configDataRoutes = require("./routes/configDataRoutes");
const deviceRoutes = require("./routes/deviceRoutes");
const stateRoutes = require("./routes/stateRoutes");
const { collection } = require("./models/users");
const moment = require("moment-timezone");
const typeModalRoutes = require("./routes/typeModelRoutes");

/*------------------------------Api---------------------*/

let dateHCM = moment.tz(Date.now(), "Asia/Ho_Chi_Minh");
console.log(dateHCM.toString());
// express app
const app = express();

const connectDB = async () => {
  try {
    await mongoose.connect(process.env.DB_URL, {
      useNewUrlParser: true,
      useUnifiedTopology: true,
      socketTimeoutMS: 30000,
      keepAlive: true,
    });

    console.log("MongoDB Connected");
  } catch (err) {
    console.log(err.message);
    process.exit(1);
  }
};
const PORT = process.env.PORT || 5000;
app.listen(PORT, () => console.log("Server on port 5000"));

connectDB();

//json
app.use(express.json());
app.use(cors());

app.use("/api/auth", authRoutes);

app.use("/api/data", dataRoutes);

app.use("/api/configdata", configDataRoutes);

app.use("/api/state", stateRoutes);

app.use("/api/device", deviceRoutes);

app.use("/api/totalvolume", totalvolumeRoutes);

app.use("/api/setvolume", setVolumeRoutes);

app.use("/api/changenutri", changeNutriRoutes);

app.use("/api/typemodal", typeModalRoutes);

/*-------------------- subcribe MQTT-----------------------*/
program
  .option(
    "-p, --protocol <type>",
    "connect protocol: mqtt, mqtts, ws, wss. default is mqtt",
    "mqtt"
  )
  .parse(process.argv);

const host = "ngoinhaiot.com";
const port = "1111";
const clientId = `mqtt_${Math.random().toString(16).slice(3)}`;

// connect options
const OPTIONS = {
  clientId,
  clean: true,
  connectTimeout: 4000,
  username: "hoangtoancsgl",
  password: "850B3436127D4E73",
  reconnectPeriod: 1000,
};
// protocol list
const PROTOCOLS = ["mqtt", "mqtts", "ws", "wss"];

// default is mqtt, unencrypted tcp connection
let connectUrl = `mqtt://${host}:${port}`;
if (program.protocol && PROTOCOLS.indexOf(program.protocol) === -1) {
  console.log("protocol must one of mqtt, mqtts, ws, wss.");
} else if (program.protocol === "mqtts") {
  // mqtts， encrypted tcp connection
  connectUrl = `mqtts://${host}:8883`;
  OPTIONS["ca"] = fs.readFileSync("./ngoinhaiot.com-ca.crt");
} else if (program.protocol === "ws") {
  // ws, unencrypted WebSocket connection
  const mountPath = "/mqtt"; // mount path, connect emqx via WebSocket
  connectUrl = `ws://${host}:8083${mountPath}`;
} else if (program.protocol === "wss") {
  // wss, encrypted WebSocket connection
  const mountPath = "/mqtt"; // mount path, connect emqx via WebSocket
  connectUrl = `wss://${host}:8084${mountPath}`;
  OPTIONS["ca"] = fs.readFileSync("./ngoinhaiot.com-ca.crt");
} else {
}

const topic = "#";

const client = mqtt.connect(connectUrl, OPTIONS);

client.on("connect", () => {
  console.log(`${program.protocol}: Connected`);
  client.subscribe([topic], () => {
    console.log(`${program.protocol}: Subscribe to topic '${topic}'`);
  });

  client.publish(
    "hoangtoancsgl/test_mqtt_node_js",
    "ok",
    { qos: 0, retain: false },
    (error) => {
      if (error) {
        console.error(error);
      }
    }
  );
});

// get new device

const saveToDatabase = function (
  dataName,
  deviceId,
  PHvalue,
  TDSvalue,
  tempvalue,
  PH_set,
  TDS_set,
  PH_dead,
  TDS_dead,
  Nutri_A,
  Nutri_B,
  ratio,
  Water,
  Acid_So,
  Base_So,
  ChangeNutri
) {
  MongoClient.connect(
    process.env.DB_URL,
    {
      useNewUrlParser: true,
      useUnifiedTopology: true,
      socketTimeoutMS: 30000,
      keepAlive: true,
    },
    (err, db) => {
      if (err) throw err;
      const dbo = db.db("test");
      //check new device
      dbo
        .collection("newdevices")
        .findOne({ device: deviceId }, (error, res) => {
          if (error) throw error;
          if (res === null) {
            console.log("New device Id: %s has been found", deviceId);

            const myobj1 = {
              device: deviceId,
              lastOnline: dateHCM._d.toUTCString(),
            };
            dbo.collection("newdevices").insertOne(myobj1, (error, res) => {
              if (error) throw error;
              console.log("Add an unauthenticate device: ", deviceId);
            });
          }
          //add new device
        });
      console.log(moment.tz(Date.now(), "Asia/Ho_Chi_Minh"));
      if (dataName === "datas") {
        dbo
          .collection("devices")
          .findOne({ device: deviceId }, (error, res) => {
            if (error) throw error;
            if (res !== null) {
              dateHCM = moment.tz(Date.now(), "Asia/Ho_Chi_Minh");
              const myobj = {
                device: deviceId,
                temperature: tempvalue,
                TDS: TDSvalue,
                PH: PHvalue,
                PH_set: PH_set,
                TDS_set: TDS_set,
                PH_dead: PH_dead,
                TDS_dead: TDS_dead,
                ratio: ratio,
                user: res.user,
                createdAt: dateHCM._d.toUTCString(),
              };
              dbo.collection(dataName).insertOne(myobj, (error, res) => {
                if (error) throw error;
              });
            }
          });
        dbo
          .collection("devices")
          .findOne({ device: deviceId }, (error, res) => {
            if (error) throw error;
            if (res !== null) {
              dateHCM = moment.tz(Date.now(), "Asia/Ho_Chi_Minh");
              const myobj = {
                device: deviceId,
                Nutri_A: Nutri_A,
                Nutri_B: Nutri_B,
                Water: Water,
                Acid_So: Acid_So,
                Base_So: Base_So,
                user: res.user,
                createdAt: dateHCM._d.toUTCString(),
              };
              dbo.collection("volume").insertOne(myobj, (error, res) => {
                if (error) throw error;
              });
            }
          });
        if (ChangeNutri === "1") {
          dbo
            .collection("changnutris")
            .findOne({ device: deviceId }, (error, res) => {
              if (error) throw error;
              if (res !== null) {
                dateHCM = moment.tz(Date.now(), "Asia/Ho_Chi_Minh");
                const myobj1 = {
                  $set: {
                    device: deviceId,
                    Nutri_A: "0",
                    Nutri_B: "0",
                    Water: "0",
                    Acid_So: "0",
                    Base_So: "0",
                    user: res.user,
                    createdAt: dateHCM._d.toUTCString(),
                  },
                };
                dbo
                  .collection("changnutris")
                  .findOneAndUpdate(
                    { device: deviceId },
                    myobj1,
                    (error, res) => {
                      if (error) throw error;
                    }
                  );
              }
            });
          dbo
            .collection("totalvolumes")
            .findOne({ device: deviceId }, (error, res) => {
              if (error) throw error;
              if (res !== null) {
                dateHCM = moment.tz(Date.now(), "Asia/Ho_Chi_Minh");
                const myobj12 = {
                  $set: {
                    device: deviceId,
                    Nutri_A: "0",
                    Nutri_B: "0",
                    Water: "0",
                    Acid_So: "0",
                    Base_So: "0",
                    user: res.user,
                    createdAt: dateHCM._d.toUTCString(),
                  },
                };
                dbo
                  .collection("totalvolume")
                  .findOneAndUpdate(
                    { device: deviceId },
                    myobj12,
                    (error, res) => {
                      if (error) throw error;
                    }
                  );
              }
            });
        }

        dbo
          .collection("devices")
          .findOne({ device: deviceId }, (error, res) => {
            if (error) throw error;
            if (res !== null) {
              dbo
                .collection("totalvolumes")
                .findOne({ device: deviceId }, (error, responce) => {
                  if (responce == null) {
                    dateHCM = moment.tz(Date.now(), "Asia/Ho_Chi_Minh");
                    const myobj = {
                      device: deviceId,
                      Nutri_A: Nutri_A,
                      Nutri_B: Nutri_B,
                      Water: Water,
                      Acid_So: Acid_So,
                      Base_So: Base_So,
                      user: res.user,
                      createdAt: dateHCM._d.toUTCString(),
                    };
                    dbo
                      .collection("totalvolumes")
                      .insertOne(myobj, (error, res) => {
                        if (error) throw error;
                      });
                  } else
                    dbo
                      .collection("totalvolumes")
                      .findOne({ device: deviceId }, (error, resp) => {
                        const myobj = {
                          $set: {
                            device: deviceId,
                            Nutri_A: (
                              parseFloat(resp.Nutri_A) + parseFloat(Nutri_A)
                            ).toString(),
                            Nutri_B: (
                              parseFloat(resp.Nutri_B) + parseFloat(Nutri_B)
                            ).toString(),
                            Water: (
                              parseFloat(resp.Water) + parseFloat(Water)
                            ).toString(),
                            Acid_So: (
                              parseFloat(resp.Acid_So) + parseFloat(Acid_So)
                            ).toString(),
                            Base_So: (
                              parseFloat(resp.Base_So) + parseFloat(Base_So)
                            ).toString(),
                            user: res.user,
                            createdAt: dateHCM._d.toUTCString(),
                          },
                        };
                        dbo
                          .collection("totalvolumes")
                          .findOneAndUpdate(
                            { device: deviceId },
                            myobj,
                            (error, res) => {
                              if (error) throw error;
                            }
                          );
                        dbo
                          .collection("changnutris")
                          .findOne({ device: deviceId }, (error, respon) => {
                            if (respon == null) {
                              dateHCM = moment.tz(
                                Date.now(),
                                "Asia/Ho_Chi_Minh"
                              );
                              const myobj1 = {
                                device: deviceId,
                                Nutri_A: "0",
                                Nutri_B: "0",
                                Water: "0",
                                Acid_So: "0",
                                Base_So: "0",
                                user: res.user,
                                createdAt: dateHCM._d.toUTCString(),
                              };
                              dbo
                                .collection("changnutris")
                                .insertOne(myobj1, (error, res) => {
                                  if (error) throw error;
                                });
                            }
                          });
                        dbo
                          .collection("setvolumes")
                          .findOne({ device: deviceId }, (error, responces) => {
                            if (responces !== null) {
                              if (
                                myobj.$set.Nutri_A >=
                                parseFloat(responces.Nutri_A_full)
                              ) {
                                updateStatusBottle(deviceId, 0);
                              }
                              if (
                                myobj.$set.Nutri_B >=
                                parseFloat(responces.Nutri_B_full)
                              ) {
                                updateStatusBottle(deviceId, 1);
                              }
                              if (
                                myobj.$set.Water >=
                                parseFloat(responces.Water_full)
                              ) {
                                updateStatusBottle(deviceId, 2);
                              }
                              if (
                                myobj.$set.Acid_So >=
                                parseFloat(responces.Acid_So_full)
                              ) {
                                updateStatusBottle(deviceId, 3);
                              }
                              if (
                                myobj.$set.Base_So >=
                                parseFloat(responces.Base_So_full)
                              ) {
                                updateStatusBottle(deviceId, 4);
                              }
                            }
                          });
                      });

                  if (error) throw error;
                });
            }
          });
      }
    }
  );
};

function updateStatusBottle(deviceId, offset) {
  MongoClient.connect(
    process.env.DB_URL,
    {
      useNewUrlParser: true,
      useUnifiedTopology: true,
      socketTimeoutMS: 30000,
      keepAlive: true,
    },
    (err, db) => {
      if (err) throw err;
      const dbo = db.db("test");

      dbo
        .collection("changnutris")
        .findOne({ device: deviceId }, (error, respon) => {
          switch (offset) {
            case 0:
              respon.Nutri_A = "1";
              break;
            case 1:
              respon.Nutri_B = "1";
              break;
            case 2:
              respon.Water = "1";
              break;
            case 3:
              respon.Acid_So = "1";
              break;
            case 4:
              respon.Base_So = "1";
              break;
            default:
          }
          dateHCM = moment.tz(Date.now(), "Asia/Ho_Chi_Minh");
          const myobj = {
            $set: {
              device: deviceId,
              Nutri_A: respon.Nutri_A,
              Nutri_B: respon.Nutri_B,
              Water: respon.Water,
              Acid_So: respon.Acid_So,
              Base_So: respon.Base_So,
              user: respon.user,
              createdAt: dateHCM._d.toUTCString(),
            },
          };
          dbo
            .collection("changnutris")
            .findOneAndUpdate({ device: deviceId }, myobj, (error, res) => {
              if (error) throw error;
            });
        });
    }
  );
}

// save state
function createDateAsUTC(date) {
  return new Date(
    Date.UTC(
      date.getFullYear(),
      date.getMonth(),
      date.getDate(),
      date.getHours(),
      date.getMinutes(),
      date.getSeconds()
    )
  );
}
const saveState = function (deviceId, state) {
  var today = new Date();

  MongoClient.connect(
    process.env.DB_URL,
    {
      useNewUrlParser: true,
      useUnifiedTopology: true,
      socketTimeoutMS: 30000,
      keepAlive: true,
    },
    (err, db) => {
      if (err) throw err;
      const dbo = db.db("test");
      //check new device
      dbo.collection("devices").findOne({ device: deviceId }, (error, res) => {
        if (error) throw error;
        if (res !== null) {
          const myobj1 = {
            device: deviceId,
            state: state,
            user: res.user,
            createdAt: dateHCM._d.toUTCString(),
          };
          dbo.collection("states").insertOne(myobj1, (error, res) => {
            if (error) throw error;
          });
        }
      });
    }
  );
};

//MQTT

client.on("reconnect", (error) => {
  console.log(`Reconnecting(${program.protocol}):`, error);
});

client.on("error", (error) => {
  console.log(`Cannot connect(${program.protocol}):`, error);
});
let PH,
  TDS,
  TEMP,
  PH_set,
  TDS_set,
  PH_dead,
  TDS_dead,
  Nutri_A,
  Nutri_B,
  ratio,
  Water,
  Acid_So,
  Base_So,
  ChangeNutri;
client.on("message", (topic, payload) => {
  console.log("Received Message:", topic, payload.toString());
  let device = topic.split("/");
  var regex = /[+-]?\d+(\.\d+)?/g;
  let data = payload.toString().match(regex);
  if (device[2] === "status") {
    saveState(device[1], payload.toString());
  } else if (device[2] === "data") {
    if (data !== null) {
      PH = data[0];
      TDS = data[1];
      TEMP = data[2];
      PH_set = data[3];
      TDS_set = data[4];
      PH_dead = data[5];
      TDS_dead = data[6];
      Nutri_A = data[7];
      Nutri_B = data[8];
      ratio = data[9];
      Acid_So = data[10];
      Base_So = data[11];
      Water = data[12];
      ChangeNutri = data[13];
      saveToDatabase(
        "datas",
        device[1],
        PH,
        TDS,
        TEMP,
        PH_set,
        TDS_set,
        PH_dead,
        TDS_dead,
        Nutri_A,
        Nutri_B,
        ratio,
        Water,
        Acid_So,
        Base_So,
        ChangeNutri
      );
    }
  }
});

/*---------------------------socket-io-------------------------*/

const io = require("socket.io")(5200, {
  cors: {
    origin: process.env.BACKEND_URL,
  },
});

let users = [];

const addUser = (userId, socketId) => {
  !users.some((user) => user.userId === userId) &&
    users.push({ userId, socketId });
  console.log(userId, socketId);
};

const getUser = (userId) => {
  return users.find((user) => user.userId === userId);
};

const removeUser = (socketId) => {
  users = users.filter((user) => user.socketId !== socketId);
};

// get last data
const getlastdata = function (userID) {};

io.on("connection", (socket) => {
  // connect
  console.log("A user connected");
  //get userid and socketid
  socket.on("addUser", (userId) => {
    addUser(userId, socket.id);
    io.emit("getUsers", users); //to all user
  });
  let changeStream;
  //send data and get control data
  socket.on("getData", (userId) => {
    const user = getUser(userId);

    MongoClient.connect(
      process.env.DB_URL,
      { useNewUrlParser: true, useUnifiedTopology: true },
      (err, db) => {
        if (err) throw err;
        const dbo = db.db("test");
        //check new device
        changeStream = dbo.collection("datas").watch();
        changeStream.on("change", (change) => {
          if (change.operationType) {
            // console.log(change.fullDocument.user._id.toString());
            if (change.fullDocument.user._id.toString() === userId) {
              const data1 = {
                _id: change.fullDocument._id,
                device: change.fullDocument.device,
                temperature: change.fullDocument.temperature,
                TDS: change.fullDocument.TDS,
                PH: change.fullDocument.PH,
                user: change.fullDocument.user,
                createdAt: change.fullDocument.createdAt,
              };
              io.to(user.socketId).emit("sendData", data1);
              console.log(data1);
            }
          }
        });
      }
    );
  });

  socket.on("getState", (userId) => {
    const user = getUser(userId);

    MongoClient.connect(
      process.env.DB_URL,
      { useNewUrlParser: true, useUnifiedTopology: true },
      (err, db) => {
        if (err) throw err;
        const dbo = db.db("test");
        //check new device
        changeStream = dbo.collection("states").watch();
        changeStream.on("change", (change) => {
          if (change.operationType) {
            const state = {
              _id: change.fullDocument._id,
              device: change.fullDocument.device,
              state: change.fullDocument.state,
              user: change.fullDocument.user,
              createdAt: change.fullDocument.createdAt,
            };
            io.to(user.socketId).emit("sendState", state);
            console.log(state);
          }
        });
      }
    );
  });

  socket.on("getConfig", (userId) => {
    const user = getUser(userId);

    MongoClient.connect(
      process.env.DB_URL,
      { useNewUrlParser: true, useUnifiedTopology: true },
      (err, db) => {
        if (err) throw err;
        const dbo = db.db("test");
        //check new device
        changeStream = dbo.collection("config-datas").watch();
        changeStream.on("change", (change) => {
          if (change.operationType) {
            if (change.fullDocument.user._id.toString() === userId) {
              const config = {
                device: change.fullDocument.device,
                TDS_val: change.fullDocument.TDS,
                TDS_dead: change.fullDocument.dead_TDS,
                PH_val: change.fullDocument.PH,
                PH_dead: change.fullDocument.dead_PH,
              };
              io.to(user.socketId).emit("sendConfig", config);
              console.log(config);
              const config_json = JSON.stringify(config);
              console.log(typeof config_json);
              const string = "hoangtoancsgl/" + config.device + "/config";
              client.publish(
                string,
                config_json,
                { qos: 0, retain: false },
                (error) => {
                  if (error) {
                    console.error(error);
                  }
                }
              );
            }
          }
        });
      }
    );
  });

  //disconnect
  socket.on("disconnect", () => {
    console.log("A user disconnected");
    removeUser(socket.id);
  });
});


