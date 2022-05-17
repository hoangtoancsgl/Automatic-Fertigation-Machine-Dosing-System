const express = require("express");
const router = express.Router();
const verifyToken = require("../middleware/auth");

const changeNutri = require("../models/changeNutri");

router.get("/:device", verifyToken, async (req, res) => {
  try {
    const getalldata = await changeNutri
      .findOne({ user: req.userId, device: req.params.device })
      .sort({ _id: -1 })
      .limit(1);
    res.json({ success: true, getalldata });
  } catch (error) {
    console.log(error);
    res.status(500).json({ success: false, message: "Internal server error" });
  }
});
router.post("/", verifyToken, async (req, res) => {
  const { device, Nutri_A, Nutri_B, Water, Acid_So, Base_So } = req.body;
  try {
    const newadjust = new changeNutri({
      device,
      Nutri_A,
      Nutri_B,
      Water,
      Acid_So,
      Base_So,
      user: req.userId,
    });
    await newadjust.save();
    res.json({
      success: true,
      message: "New updates success",
      post: newadjust,
    });
  } catch (error) {
    console.log(error);
    res.status(500).json({ success: false, message: "Internal server error" });
  }
});
module.exports = router;
