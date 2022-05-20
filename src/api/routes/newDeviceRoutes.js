const express = require("express");
const router = express.Router();
const verifyToken = require("../middleware/auth");

const newDevices = require("../models/newDevices");

// @route GET api/device
// @Get the last data
// @access private
router.get("/", async (req, res) => {
  try {
    const getalldata = await newDevices.find();

    res.json({ success: true, getalldata });
  } catch (error) {
    console.log(error);
    res.status(500).json({ success: false, message: "Internal server error" });
  }
});

module.exports = router;
