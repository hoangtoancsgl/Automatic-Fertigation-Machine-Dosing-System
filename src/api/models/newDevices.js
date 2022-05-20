const mongoose = require("mongoose");
const moment = require("moment-timezone");
const Schema = mongoose.Schema;

const newDevicesSchema = new Schema({
  device: {
    type: String,
    required: true,
  },
  lastOnline: {
    type: String,
    default: moment.tz(Date.now(), "Asia/Ho_Chi_Minh")._d,
  },
});
module.exports = mongoose.model("newDevices", newDevicesSchema);
