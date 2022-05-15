const express = require("express");
const router = express.Router();
const verifyToken = require("../middleware/auth");

const configData = require("../models/configData");
const configType = require("../models/configType");
// @route GET api/config_data
// @Get the last data
// @access private

router.get("/", verifyToken, async (req, res) => {
  try {
    const getLastConfigData = await configData
      .findOne({ user: req.userId })
      .sort({ _id: -1 })
      .limit(1);
    if (getLastConfigData === null) {
      res.json({
        success: true,
        getLastConfigData: {
          TDS: 0,
          dead_TDS: 0,
          PH: 0,
          dead_PH: 0,
          nutri_Ratio: 0,
          user: req.userId,
        },
      });
    } else res.json({ success: true, getLastConfigData });
  } catch (error) {
    console.log(error);
    res.status(500).json({ success: false, message: "Internal server error" });
  }
});

// @route POST api/configdata
// @your newadjust
// @access private
router.post("/", verifyToken, async (req, res) => {
  const { device, type, TDS, dead_TDS, PH, dead_PH, nutri_Ratio } = req.body;
  try {
    const newadjust = new configData({
      device,
      TDS,
      type,
      dead_TDS,
      PH,
      dead_PH,
      nutri_Ratio,
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

// @route GET api/configdata/configtype
// @Get the last data
// @access private

router.get("/configtype", verifyToken, async (req, res) => {
  try {
    const getLastConfigType = await configType.find({ user: req.userId });

    if (getLastConfigType === null) {
      res.json({
        success: true,
        getlastconfigType: {
          type: "",
          TDS: 0,
          dead_TDS: 0,
          PH: 0,
          dead_PH: 0,
          nutri_Ratio: 0,
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

router.post("/configtype", verifyToken, async (req, res) => {
  const { type, TDS, dead_TDS, PH, dead_PH, nutri_Ratio } = req.body;
  try {
    const newadjust = new configType({
      type,
      TDS,
      dead_TDS,
      PH,
      dead_PH,
      nutri_Ratio,
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

// @route update api/configdata/configtype
// @update data
// @access private
router.put("/configtype/:id", verifyToken, async (req, res) => {
  const { type, TDS, dead_TDS, PH, dead_PH, nutri_Ratio } = req.body;
  try {
    let updateadjust = {
      type,
      TDS,
      dead_TDS,
      PH,
      dead_PH,
      nutri_Ratio,
      user: req.userId,
    };
    console.log(updateadjust);
    const UpdateCondion = { _id: req.params.id, user: req.userId };

    updateadjust = await configType.findOneAndUpdate(
      UpdateCondion,
      updateadjust,
      {
        new: true,
      }
    );

    //user not authorised to update
    if (!updateadjust)
      return res.status(401).json({
        success: false,
        message: "Post not Found or user not authorised",
      });
    res.json({
      success: true,
      message: "updates successfull",
      post: updateadjust,
    });
  } catch (error) {
    console.log(error);
    res.status(500).json({ success: false, message: "Internal server error" });
  }
});

// @route DELETE api/configdata/configtype
// @delete data
// @access private
router.delete("/configtype/:id", verifyToken, async (req, res) => {
  try {
    const postDeleleCondition = { _id: req.params.id, user: req.userId };
    const deleteData = await configType.findOneAndDelete(postDeleleCondition);

    //user not authoried or post not found
    if (!deleteData)
      return res.status(401).json({
        success: false,
        message: "Type not found or user not authoried",
      });
    res.json({ success: true, post: deleteData });
  } catch (error) {
    console.log(error);
    res.status(500).json({ success: false, message: "Internal server error" });
  }
});

module.exports = router;
