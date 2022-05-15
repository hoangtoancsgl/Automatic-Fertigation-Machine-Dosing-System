const mongoose = require("mongoose");
const Schema = mongoose.Schema;
const moment = require("moment-timezone");
const setVolumeSchema = new Schema({
  device: {
    type: String,
    required: false,
  },
  Nutri_A_full: {
    type: String,
    required: true,
  },
  Nutri_B_full: {
    type: String,
    required: true,
  },
  Water_full: {
    type: String,
    required: true,
  },
  Acid_So_full: {
    type: String,
    required: true,
  },
  Base_So_full: {
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
module.exports = mongoose.model("setVolume", setVolumeSchema);
