const express = require("express");
const router = express.Router();
const verifyToken = require("../middleware/auth");

const configData = require("../models/configData");
const data = require("../models/data");

// @route GET api/data
// @Get the last data
// @access private

router.get("/:device", verifyToken, async (req, res) => {
  try {
    const getalldata = await data
      .findOne({ user: req.userId, device: req.params.device })
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
router.get("/chart/:device/:time", verifyToken, async (req, res) => {
  const arrayTemp = [];
  const arrayTDS = [];
  const arrayPH = [];
  const arrayTime = [];
  let seriesTemp = [];
  let seriesTDS = [];
  let seriesPH = [];
  try {
    const getChart = await data
      .find({ user: req.userId, device: req.params.device })

      .sort({ _id: -1 })
      .limit(req.params.time);
    const getFullDataChart = await data.find({
      user: req.userId,
      device: req.params.device,
    });
    // console.log(getFullDataChart.length);
    if (Object.keys(getChart).length === 0) {
      console.log(Object.keys(getChart).length);
      var getalldata = [seriesTemp, seriesTDS, seriesPH];
      res.json({
        success: true,
        getalldata,
      });
    } else {
      let x = req.params.time;
      if (getFullDataChart.length < req.params.time)
        x = getFullDataChart.length;
      console.log(x);
      for (let j = 0; j < x; j++) {
        var date = new Date(getChart[j].createdAt);
        arrayTime.push(date.getTime());
        arrayTemp.push(getChart[j].temperature * 1);
        arrayTDS.push(getChart[j].TDS * 1);
        arrayPH.push(getChart[j].PH * 1);
      }
      var values = [arrayTemp, arrayTDS, arrayPH, arrayTime];
      var i = 0;
      while (i < x) {
        seriesTemp.push([values[3][i], values[0][i]]);
        seriesTDS.push([values[3][i], values[1][i]]);
        seriesPH.push([values[3][i], values[2][i]]);
        i++;
      }
      var getalldata = [seriesTemp, seriesTDS, seriesPH, arrayTime];
      // console.log(getalldata);
      res.json({ success: true, getalldata });
    }
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
