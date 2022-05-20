const express = require("express");
const router = express.Router();
const verifyToken = require("../middleware/auth");

const returnAck = require("../models/returnAck");
// @route GET api/device
// @Get the last data
// @access private
router.get("/", verifyToken, async (req, res) => {
  try {
    const getalldata = await returnAck.find({ user: req.users });

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
  const { device, replyAck } = req.body;
  console.log(req.body);
  try {
    const newadjust = new returnAck({
      device,
      replyAck,
      user: req.userId,
    });
    await newadjust.save();
    res.json({
      success: true,
      message: "return ACK added successfully",
      post: newadjust,
    });
  } catch (error) {
    console.log(error);
    res.status(500).json({ success: false, message: "Internal server error" });
  }
});

module.exports = router;
