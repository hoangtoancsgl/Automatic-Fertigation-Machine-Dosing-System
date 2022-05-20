const mongoose = require("mongoose");
const Schema = mongoose.Schema;
const returnAckSchema = new Schema({
  device: {
    type: String,
    required: true,
  },
  replyAck: {
    type: String,
    required: true,
  },
  user: {
    type: Schema.Types.ObjectId,
    ref: "users",
  },
  createdAt: {
    type: String,
    default: Date.now(),
  },
});
module.exports = mongoose.model("returnAck", returnAckSchema);
