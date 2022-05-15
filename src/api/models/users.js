const mongoose = require("mongoose");
const Schema = mongoose.Schema;
const moment = require("moment-timezone");

const UserSchema = new Schema({
  username: {
    type: String,
    require: true,
    unique: true,
  },
  password: {
    type: String,
    required: true,
  },
  createdAt: {
    type: Date,
    default: moment.tz(Date.now(), "Asia/Ho_Chi_Minh")._d,
  },
});

module.exports = mongoose.model("users", UserSchema);
