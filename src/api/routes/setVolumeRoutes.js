const express = require("express");
const router = express.Router();
const verifyToken = require("../middleware/auth");

const setVolume = require("../models/setVolume");

router.get("/", verifyToken, async (req, res) => {
  try {
    const getLastConfigType = await setVolume.find({ user: req.userId });

    if (getLastConfigType === null) {
      res.json({
        success: true,
        getlastconfigType: {
          device: "",
          Nutri_A_full: 0,
          Nutri_B_full: 0,
          Water_full: 0,
          Acid_So_full: 0,
          Base_So_full: 0,
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
    device,
    Nutri_A_full,
    Nutri_B_full,
    Water_full,
    Acid_So_full,
    Base_So_full,
  } = req.body;
  try {
    const newadjust = new setVolume({
      device,
      Nutri_A_full,
      Nutri_B_full,
      Water_full,
      Acid_So_full,
      Base_So_full,
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
router.put("/:id", verifyToken, async (req, res) => {
  const {
    device,
    Nutri_A_full,
    Nutri_B_full,
    Water_full,
    Acid_So_full,
    Base_So_full,
  } = req.body;
  try {
    let updateadjust = {
      device,
      Nutri_A_full,
      Nutri_B_full,
      Water_full,
      Acid_So_full,
      Base_So_full,
      user: req.userId,
    };
    console.log(updateadjust);
    const UpdateCondion = { _id: req.params.id, user: req.userId };

    updateadjust = await setVolume.findOneAndUpdate(
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

module.exports = router;
