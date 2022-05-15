const mongoose = require("mongoose");
const Schema = mongoose.Schema;
const moment = require("moment-timezone");

const stateSchema = new Schema({
  device: {
    type: String,
    required: true,
  },
  status: {
    type: String,
    require: true,
  },
  user: {
    type: Schema.Types.ObjectId,
    ref: "users",
  },
  createdAt: {
    type: Date,
    default: moment.tz(Date.now(), "Asia/Ho_Chi_Minh")._d,
  },
});
module.exports = mongoose.model("state", stateSchema);
