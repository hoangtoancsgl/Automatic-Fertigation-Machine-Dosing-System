const express = require("express");
const router = express.Router();
const verifyToken = require("../middleware/auth");

const devices = require("../models/devices");

// @route GET api/device
// @Get the last data
// @access private
router.get("/", verifyToken, async (req, res) => {
  try {
    const getalldata = await data
      .findOne({ user: req.userId })
      .sort({ _id: -1 })
      .limit(1);
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
  const { device } = req.body;
  try {
    const newadjust = new devices({
      device,
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
