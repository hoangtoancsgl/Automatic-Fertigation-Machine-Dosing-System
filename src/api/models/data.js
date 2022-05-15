const mongoose = require("mongoose");
const Schema = mongoose.Schema;
const moment = require("moment-timezone");
const dataSchema = new Schema({
  device: {
    type: String,
    required: false,
  },
  temperature: {
    type: String,
    required: true,
  },
  TDS: {
    type: String,
    required: true,
  },
  PH: {
    type: String,
    required: true,
  },
  user: {
    type: Schema.Types.ObjectId,
    ref: "users",
  },
  createdAt: {
    type: String,
    default: moment.tz(Date.now(), "Asia/Ho_Chi_Minh")._d,
  },
});
module.exports = mongoose.model("Data", dataSchema);
