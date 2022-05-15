const mongoose = require("mongoose");
const Schema = mongoose.Schema;

const configModelSchema = new Schema({
  typeModal: {
    type: String,
    required: true,
  },
  // gradation: {
  //   type: String,
  //   required: true,
  // },
  TDSModal: {
    type: String,
    required: true,
  },
  dead_TDSModal: {
    type: String,
    required: true,
  },
  PHModal: {
    type: String,
    required: true,
  },
  dead_PHModal: {
    type: String,
    required: true,
  },
  nutri_RatioModal: {
    type: String,
    required: true,
  },
});
module.exports = mongoose.model("configTypeModel", configModelSchema);
