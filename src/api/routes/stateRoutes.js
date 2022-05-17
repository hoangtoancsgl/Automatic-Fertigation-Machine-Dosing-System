const express = require("express");
const router = express.Router();
const verifyToken = require("../middleware/auth");

const state = require("../models/state");
const data = require("../models/data");
// @route GET api/state
// @Get the last data
// @access private

router.get("/:device", verifyToken, async (req, res) => {
  try {
    const getState = await state
      .findOne({ user: req.userId, device: req.params.device })
      .sort({ _id: -1 })
      .limit(1);
    if (getState === null) {
      res.json({
        success: true,
        getState: {
          status: "offline",
          user: req.userId,
        },
      });
    } else res.json({ success: true, getState });
  } catch (error) {
    console.log(error);
    res.status(500).json({ success: false, message: "Internal server error" });
  }
});
module.exports = router;
