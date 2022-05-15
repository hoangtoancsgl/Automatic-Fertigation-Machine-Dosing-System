const express = require("express");
const router = express.Router();
const verifyToken = require("../middleware/auth");

const configData = require("../models/configData");
const data = require("../models/data");

// @route GET api/data
// @Get the last data
// @access private
router.get("/", verifyToken, async (req, res) => {
  try {
    const getalldata = await data
      .findOne({ user: req.userId })
      .sort({ _id: -1 })
      .limit(1);
    if (getalldata === null) {
      res.json({
        success: true,
        getalldata: {
          temperature: 0,
          TDS: 0,
          PH: 0,
          user: req.userId,
        },
      });
    } else res.json({ success: true, getalldata });
  } catch (error) {
    console.log(error);
    res.status(500).json({ success: false, message: "Internal server error" });
  }
});

// @route GET api/chart
// @Get last 5 datas
// @access private
router.get("/chart", verifyToken, async (req, res) => {
  try {
    const getalldata = await data
      .find({ user: req.userId })
      .sort({ _id: -1 })
      .limit(1440);
    if (getalldata === null) {
      res.json({
        success: true,
        getalldata: {
          temperature: 0,
          TDS: 0,
          PH: 0,
          user: req.userId,
        },
      });
    } else res.json({ success: true, getalldata });
  } catch (error) {
    console.log(error);
    res.status(500).json({ success: false, message: "Internal server error" });
  }
});

// @route update api/data
// @update data
// @access private
router.put("/:id", verifyToken, async (req, res) => {
  const {
    device,
    temperature,
    DTS,
    PH,
    PH_set,
    TDS_set,
    PH_dead,
    TDS_dead,
    Nutri_A,
    Nutri_B,
    water,
  } = req.body;
  try {
    let updateadjust = {
      device,
      temperature,
      DTS,
      PH,
      PH_set,
      TDS_set,
      PH_dead,
      TDS_dead,
      Nutri_A,
      Nutri_B,
      water,
      user: req.userId,
    };

    const UpdateCondion = { _id: req.params.id, user: req.userId };

    updateadjust = await data.findOneAndUpdate(UpdateCondion, updateadjust, {
      new: true,
    });

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

// @route DELETE api/data
// @delete data
// @access private
router.delete("/:id", verifyToken, async (req, res) => {
  try {
    const postDeleleCondition = { _id: req.params.id, user: req.userId };
    const deleteData = await data.findOneAndDelete(postDeleleCondition);

    //user not authoried or post not found
    if (!deleteData)
      return res.status(401).json({
        success: false,
        message: "Post not found or user not authoried",
      });
    res.json({ success: true, post: deleteData });
  } catch (error) {
    console.log(error);
    res.status(500).json({ success: false, message: "Internal server error" });
  }
});
module.exports = router;
