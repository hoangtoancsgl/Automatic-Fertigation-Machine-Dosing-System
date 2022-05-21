const express = require("express");
const router = express.Router();
const verifyToken = require("../middleware/auth");

const devices = require("../models/devices");
const newDevices = require("../models/newDevices");
// @route GET api/device
// @Get the last data
// @access private
router.get("/", verifyToken, async (req, res) => {
  try {
    const getalldata = await devices.find({ user: req.userId });

    res.json({ success: true, getalldata });
  } catch (error) {
    console.log(error);
    res.status(500).json({ success: false, message: "Internal server error" });
  }
});

// @route POST api/device
// @your newadjust
// @access private
router.post("/", verifyToken, async (req, res) => {
  const { device, name } = req.body;
  try {
    //Check for exsisting user
    const connectedDevice = await newDevices.findOne({ device });
    if (connectedDevice) {
      const existingDevice = await devices.findOne({
        device,
      });

      if (existingDevice)
        if (existingDevice.user.toString() === req.userId) {
          return res.status(400).json({
            success: false,
            message: "Device have already added",
          });
        } else {
          return res.status(400).json({
            success: false,
            message: "Invalid device",
          });
        }
    } else {
      return res.status(400).json({
        success: false,
        message: "Device haven't online or invalid device Id",
      });
    }
    const newadjust = new devices({
      device,
      name,
      user: req.userId,
    });
    await newadjust.save();
    res.json({
      success: true,
      message: "Device added successfully",
      post: newadjust,
    });
  } catch (error) {
    console.log(error);
    res.status(500).json({ success: false, message: "Internal server error" });
  }
});

module.exports = router;
