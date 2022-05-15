const express = require("express");
const router = express.Router();
const verifyToken = require("../middleware/auth");

const configTypeModel = require("../models/configTypeModel");

router.get("/", verifyToken, async (req, res) => {
  try {
    const getLastConfigType = await configTypeModel.find();
    if (getLastConfigType === null) {
      res.json({
        success: true,
        getlastconfigType: {
          typeModal: "",
          // gradation: "",
          TDSModal: 0,
          dead_TDSModal: 0,
          PHModal: 0,
          dead_PHModal: 0,
          nutri_RatioModal: 0,
        },
      });
    } else res.json({ success: true, getLastConfigType });
  } catch (error) {
    console.log(error);
    res.status(500).json({ success: false, message: "Internal server error" });
  }
});

// @route POST api/configdata/configtype
// @your newadjust
// @access private

router.post("/", verifyToken, async (req, res) => {
  const {
    typeModal,
    // gradation,
    TDSModal,
    dead_TDSModal,
    PHModal,
    dead_PHModal,
    nutri_RatioModal,
  } = req.body;
  try {
    const newadjust = new configTypeModel({
      typeModal,
      // gradation,
      TDSModal,
      dead_TDSModal,
      PHModal,
      dead_PHModal,
      nutri_RatioModal,
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
