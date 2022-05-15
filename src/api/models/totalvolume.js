const mongoose = require("mongoose");
const Schema = mongoose.Schema;
const moment = require("moment-timezone");
const totalVolumeSchema = new Schema({
  device: {
    type: String,
    required: false,
  },
  Nutri_A: {
    type: String,
    required: true,
  },
  Nutri_B: {
    type: String,
    required: true,
  },
  Water: {
    type: String,
    required: true,
  },
  Acid_So: {
    type: String,
    required: true,
  },
  Base_So: {
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
module.exports = mongoose.model("totalvolume", totalVolumeSchema);
