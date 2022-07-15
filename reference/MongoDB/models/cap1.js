const mongoose = require("mongoose");

const cap1Schema = new mongoose.Schema({
    name: String,
    kids : [{type: mongoose.Schema.Types.ObjectId}]
});

module.exports = mongoose.model("Cap1", cap1Schema);
