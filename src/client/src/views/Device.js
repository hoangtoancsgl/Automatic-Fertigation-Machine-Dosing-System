import { useState, useContext, useEffect, useRef } from "react";
import "./device.css";
import { DataContext } from "../contexts/DataContext";
import { TypeContext } from "../contexts/TypeContext";
import { ConfigContext } from "../contexts/ConfigContext";
import { AuthContext } from "../contexts/AuthContext";
import { TypeModalContext } from "../contexts/TypeModalContext";
import AlertMessage from "../components/layout/AlertMessage";
import devicelogo from "../assets/device.jpg";
import PHImage from "../assets/PH.png";
import React from "react";

import { io } from "socket.io-client";
import Button from "react-bootstrap/Button";
import Tooltip from "react-bootstrap/Tooltip";
import OverlayTrigger from "react-bootstrap/OverlayTrigger";
import ActionButtons from "../type/ActionButton";
import addButton from "../assets/addButton.png";
import SingleType from "../type/SingleType";
import SingleTypeModal from "../type/SingleTypeModal";
import AddTypeModal from "../type/AddTypeModal";
import UpdateTypeModal from "../type/UpdateTypeModal";
import Select from "react-select";

import Toast from "react-bootstrap/Toast";
const Device = () => {
  const {
    dataState: {
      data: { device },
    },
    getData,
  } = useContext(DataContext);

  const [data, setData] = useState({ data: [] });

  const {
    typeState: { configtype, config, configLoading },
    getConfigType,
    setShowAddTypeModal,
    showToast: { show, message, Type },
    setShowToast,
  } = useContext(TypeContext);

  const {
    configState: {
      currentConfig: { type, TDS, dead_TDS, PH, dead_PH, nutri_Ratio },
    },
    getConfigData,
  } = useContext(ConfigContext);

  const {
    typeModalState: { typeModal },
    getTypeModal,
  } = useContext(TypeModalContext);

  useEffect(() => getData(), []);
  useEffect(() => getConfigData(), []);
  useEffect(() => getConfigType(), []);
  useEffect(() => getTypeModal(), []);
  // add device

  const { addDevices } = useContext(DataContext);

  const [newDevice, setNewDevice] = useState({
    device: "",
  });

  const { deviceID } = newDevice;

  const [alert, setAlert] = useState(null);

  const onChangeNewDeviceForm = (event) =>
    setNewDevice({
      ...newDevice,
      [event.target.name]: event.target.value,
    });

  const onSubmmit = async (event) => {
    event.preventDefault();
    try {
      const { message } = await addDevices(newDevice);
      setAlert({ type: "danger", message: message });
      setTimeout(() => setAlert(null), 5000);
      setNewDevice({ deviceID: "" });
    } catch (error) {
      console.log(error);
    }
  };

  const [alert_conf, setAlert_conf] = useState(null);

  const [newConfig, newSetConfig] = useState({
    TDS: "",
    dead_TDS: "",
    PH: "",
    dead_PH: "",
    nutri_Ratio: "",
  });

  const { newTDS, newDead_TDS, newPH, newDead_PH, newnutri_Ratio } = newConfig;

  const onSubmmitConf = async (event) => {
    event.preventDefault();

    try {
      const { message } = await config(newConfig);
      setAlert_conf({ type: "danger", message: message });
      setTimeout(() => setAlert_conf(null), 5000);
      newSetConfig({
        TDS: " ",
        dead_TDS: " ",
        PH: " ",
        dead_PH: " ",
        nutri_Ratio: " ",
      });
    } catch (error) {
      console.log(error);
    }
  };

  const onChangeNewSetConfigForm = (event) =>
    newSetConfig({
      ...newConfig,
      device,
      type: "Manual",
      [event.target.name]: event.target.value,
    });

  //socket io
  const {
    authState: {
      user: { username, _id },
    },
  } = useContext(AuthContext);
  const socket = useRef();

  useEffect(() => {
    socket.current = io("ws://localhost:5200", {
      reconnection: true,
      reconnectionDelay: 500,
      reconnectionAttempts: 10,
    });
    socket.current.on();
  }, []);

  useEffect(() => {
    socket.current.emit("addUser", _id);
    socket.current.on("getUsers", (users) => {
      //sconsole.log(users);
    });
    socket.current.emit("getConfig", _id);
    socket.current.on("sendConfig", (config) => {
      //console.log(config);
      setData(config);
    });
  }, [username]);

  //initialize option for personal setting
  var options = [];
  for (let i = 0; i < config.length; i++) {
    let value = [];
    value = { value: config[i]._id, label: config[i].type };
    options.push(value);
  }

  const [selectTypePersonalState, setSelectTypePersonalState] = useState("");
  const onChangePersonal = (event) => setSelectTypePersonalState(event.value);
  let body12 = null;
  if (options !== null) {
    for (let i = 0; i < config.length; i++) {
      if (options[i].value === selectTypePersonalState) {
        body12 = <SingleType config={config[i]} />;
      }
    }
  }

  //initialize option for personal setting
  var optionsGlobal = [];
  for (let i = 0; i < typeModal.length; i++) {
    let value = [];
    value = { value: typeModal[i]._id, label: typeModal[i].typeModal };
    optionsGlobal.push(value);
  }

  const [selectTypeGlobalState, setSelectTypeGlobalState] = useState("");
  const onChangeGlobal = (event) => setSelectTypeGlobalState(event.value);
  let bodyTypeGlobal = null;
  if (optionsGlobal !== null) {
    for (let i = 0; i < typeModal.length; i++) {
      if (optionsGlobal[i].value === selectTypeGlobalState) {
        bodyTypeGlobal = <SingleTypeModal typeModal={typeModal[i]} />;
      }
    }
  }

  let body1 = null;
  if (configLoading) {
    body1 = (
      <>
        <h1>Loading</h1>
      </>
    );
  } else if (config.length === 0) {
    body1 = (
      <>
        <h1 className="title-new-vegetable">
          Click button below to add a new vegetables
        </h1>
        <Button
          className="button-new-vegetable"
          variant="primary"
          onClick={setShowAddTypeModal.bind(this, true)}
        >
          Add new vegetables
        </Button>
      </>
    );
  } else {
    body1 = (
      <>
        <Select options={options} onChange={onChangePersonal} />
        {body12}

        <OverlayTrigger
          placement="left"
          overlay={<Tooltip>Add a new profile</Tooltip>}
        >
          <Button
            className="btn-floating"
            onClick={setShowAddTypeModal.bind(this, true)}
          >
            <img src={addButton} height="20" width="20" />
          </Button>
        </OverlayTrigger>
      </>
    );
  }

  let bodyGlobal = null;
  if (typeModal.length !== 0) {
    bodyGlobal = (
      <>
        <h1>Recommend profile for vegetables</h1>
        <Select options={optionsGlobal} onChange={onChangeGlobal} />
        {bodyTypeGlobal}
      </>
    );
  }

  return (
    <>
      <div className="main">
        <div className="config">
          <div className="title-new-vegetable">Current setting is {type}</div>
          {/* <h1>Config value</h1> */}

          {/* <form onSubmit={onSubmmitConf}>
            <div className="PH-config">
              <div className="pH">
                <h2>Current PH value set: {PH}</h2>
                <input
                  type="number"
                  step="0.01"
                  className="text"
                  placeholder="Set PH value..."
                  name="PH"
                  value={newPH}
                  onChange={onChangeNewSetConfigForm}
                  required
                />
              </div>
              <div className="dead_PH">
                <h2>Current dead PH value set: {dead_PH}</h2>
                <input
                  type="number"
                  step="0.01"
                  className="text"
                  placeholder="Dead PH value..."
                  name="dead_PH"
                  value={newDead_PH}
                  onChange={onChangeNewSetConfigForm}
                  required
                />
              </div>
            </div>

            <div className="TDS-config">
              <div className="tDS">
                <h2>Current TDS value set: {TDS} ppm</h2>
                <input
                  type="number"
                  step="0.01"
                  className="text"
                  placeholder="Set TDS value..."
                  name="TDS"
                  value={newTDS}
                  onChange={onChangeNewSetConfigForm}
                  required
                />
              </div>
              <div className="dead_TDS">
                <h2>Current dead TDS value set: {dead_TDS} ppm</h2>
                <input
                  type="number"
                  step="0.01"
                  className="text"
                  placeholder="Dead TDS value..."
                  name="dead_TDS"
                  value={newDead_TDS}
                  onChange={onChangeNewSetConfigForm}
                  required
                />
              </div>
            </div>
            {/* <img src={PHImage} /> */}
          {/*
            <div className="nutriRatio">
              <h2>Current nutri ratio value set: {nutri_Ratio} </h2>
              <input
                type="number"
                step="0.01"
                className="text"
                placeholder="Nutri ratio value..."
                name="nutri_Ratio"
                value={newnutri_Ratio}
                onChange={onChangeNewSetConfigForm}
                required
              />
            </div>
            <AlertMessage info={alert_conf} className="message" />
            <input type="submit" className="button" value="Submit" />
          </form> */}
          {body1}
          {bodyGlobal}
          <>
            <AddTypeModal />
            {configtype !== null && <UpdateTypeModal />}
            <Toast
              show={show}
              style={{ position: "fixed", top: "20%", right: "10px" }}
              className={`bg-${Type} text-white`}
              onClose={setShowToast.bind(this, {
                show: false,
                message: "",
                type: null,
              })}
              delay={3000}
              autohide
            >
              <Toast.Body>
                <strong>{message}</strong>
              </Toast.Body>
            </Toast>
          </>
        </div>
      </div>

      <div className="addDevice">
        <div className="device-box">
          <img src={devicelogo} />
          <div className="form">
            <h1>Add your new device</h1>
            <form onSubmit={onSubmmit}>
              <input
                type="text"
                className="text"
                placeholder="Device ID..."
                name="device"
                value={deviceID}
                onChange={onChangeNewDeviceForm}
                required
              />
              <AlertMessage info={alert} />
              <input type="submit" className="addButton" value="Add device  " />
            </form>
          </div>
        </div>
      </div>
    </>
  );
};
export default Device;
