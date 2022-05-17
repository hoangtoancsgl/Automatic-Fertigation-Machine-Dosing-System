import React, { useRef, useState } from "react";
import { AuthContext } from "../contexts/AuthContext";
import { useContext, useEffect } from "react";
import image from "../assets/iot.jpg";
import { DataContext } from "../contexts/DataContext";
import { StatusContext } from "../contexts/StatusContext";
import { TotalVolumeContext } from "../contexts/TotalVolumeContext";
import { SetVolumeContext } from "../contexts/SetVolumeContext";
import { DeviceContext } from "../contexts/DeviceContext";
import "./views.css";
import dosingmachine from "../assets/dosingmachine.png";
import { io } from "socket.io-client";

import PHicon from "../assets/PHlogo.png";
import tempIcon from "../assets/temp.png";
import tdsIcon from "../assets/tds.png";

import { CircularProgressbar, buildStyles } from "react-circular-progressbar";
import "react-circular-progressbar/dist/styles.css";
import Select from "react-select";
const Dashboard = () => {
  //get username
  const {
    authState: {
      user: { username, _id },
    },
  } = useContext(AuthContext);

  const {
    deviceState: { deviceData },
    getDevice,
  } = useContext(DeviceContext);

  const [data, setData] = useState({ data: [] });
  const {
    dataState: {
      data: { temperature, TDS, PH, createdAt },

      dataLoading,
    },
    getData,
  } = useContext(DataContext);

  const [status, setStatus] = useState({ status: [] });
  const {
    statusState: {
      status: { state },
    },
    getStatus,
  } = useContext(StatusContext);

  const {
    setVolumeState: {
      setVolume: { Nutri_A_full, Nutri_B_full, Acid_So_full, Base_So_full },
    },
    getSetVolume,
  } = useContext(SetVolumeContext);

  const {
    totalVolumeState: {
      totalVolume: { Nutri_A, Nutri_B, Water, Acid_So, Base_So },
    },
    getTotalVolume,
  } = useContext(TotalVolumeContext);

  var optionsDevice = [];
  for (let i = 0; i < deviceData.length; i++) {
    let value = [];
    value = { value: deviceData[i].device, label: `Device ${i + 1}` };
    optionsDevice.push(value);
  }
  console.log(optionsDevice);
  const [selectDevice, setselectDevice] = useState("Device 1");
  // //get last data
  useEffect(() => getDevice(), []);

  const OnChangeDevice = (event) => {
    setselectDevice(event.label);
  };
  console.log(selectDevice);
  var selectedDevice;

  for (let i = 0; i < deviceData.length; i++) {
    if (selectDevice === optionsDevice[i].label) {
      selectedDevice = deviceData[i].device;
      //console.log(selectedDevice);
    }
  }
  useEffect(() => getData(selectedDevice), [selectedDevice]);
  useEffect(() => getStatus(selectedDevice), [selectedDevice]);
  useEffect(() => getTotalVolume(selectedDevice), [selectedDevice]);
  useEffect(() => getSetVolume(selectedDevice), [selectedDevice]);
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
      console.log(users);
    });
    socket.current.emit("getData", _id);
    socket.current.on("sendData", (data) => {
      setData(data);
    });
    socket.current.emit("getState", _id);
    socket.current.on("sendState", (sendState) => {
      setStatus(sendState);
    });
  }, [username]);

  let state_style;
  if (state === "offline") {
    state_style = { backgroundColor: "grey" };
  } else if (state === "online") {
    state_style = { backgroundColor: "#4d94ff" };
  }

  var nutriA = Nutri_A_full - Nutri_A;
  var rounded;
  function getPercent(value_full, value) {
    var num = 0;
    if (value_full - value < 0) {
      num = 0;
    } else {
      num = ((value_full - value) / value_full) * 100;
    }
    var roundedString = num.toFixed(0);
    rounded = Number(roundedString);
    return rounded;
  }

  const Nutri_A_percent = getPercent(Nutri_A_full, Nutri_A);
  const Nutri_B_percent = getPercent(Nutri_B_full, Nutri_B);
  const Acid_percent = getPercent(Acid_So_full, Acid_So);
  const Base_percent = getPercent(Base_So_full, Base_So);
  var Nutri_A_left = Nutri_A_full - Nutri_A;
  var Nutri_B_left = Nutri_B_full - Nutri_B;
  var Acid_left = Acid_So_full - Acid_So;
  var Base_left = Base_So_full - Base_So;
  if (Nutri_A_left < 0) Nutri_A_left = 0;
  if (Nutri_B_left < 0) Nutri_B_left = 0;
  if (Base_left < 0) Base_left = 0;
  if (Acid_left < 0) Acid_left = 0;

  // } else Nutri_A_left = Nutri_A_full - Nutri_A;
  // if (Nutri_B_left < 0) Nutri_B_left = 0;
  // if (Acid_left < 0) Acid_left = 0;
  // if (Base_left < 0) Base_left = 0;
  // console.log(Nutri_A_percent);
  return (
    <>
      <main>
        <small className="text-mute">Last Time Update: {createdAt}</small>
        <Select
          options={optionsDevice}
          className="selecttime"
          placeholder={<div>{selectDevice}</div>}
          onChange={OnChangeDevice}
        />
        <h1 className="machine-current-state-tag">
          Current value at dosing machine
        </h1>
        <div className="insights">
          <div className="PH-TDS">
            <div className="PH">
              <img src={PHicon} className="icon-sen" />
              <div className="middle">
                <div className="left">
                  <h3>PH</h3>
                  <h1>{PH}</h1>
                </div>
                <small className="text-muted">Time update</small>
              </div>
            </div>
            <div className="TDS">
              <img src={tdsIcon} className="icon-sen" />
              <div className="middle">
                <div className="left">
                  <h3>TDS</h3>
                  <h1>{TDS}</h1>
                </div>
                <small className="text-muted">PPM</small>
              </div>
            </div>
          </div>
          <div className="temperature">
            <img src={tempIcon} className="icon-sen" />
            <div className="middle">
              <div className="left">
                <h3>Temperature</h3>
                <h1>{temperature} </h1>
                <small className="text-muted">°C</small>
              </div>
            </div>
          </div>
        </div>
        <div className="nutripercent">Current volume of nutrient at bottle</div>
        <div className="cirular-value">
          <div className="circular">
            <CircularProgressbar
              className="Nutri_A-cir"
              value={Nutri_A_left}
              maxValue={Nutri_A_full}
              text={`${Nutri_A_percent}%`}
              styles={buildStyles({
                // strokeLinecap: "butt",

                pathTransitionDuration: 0.5,

                pathColor: `#009933`,

                // trailColor: `#d6d6d6`,
                backgroundColor: "#3e98c7",
              })}
            />
            <h1>
              Nutri A:&nbsp;
              <bdo className="valueTag">{Nutri_A_left}</bdo> &nbsp;(ml)
            </h1>
          </div>

          <div className="circular">
            <CircularProgressbar
              className="Nutri_B-cir"
              value={Nutri_B_left}
              maxValue={Nutri_B_full}
              text={`${Nutri_B_percent}%`}
            />
            <h1>
              Nutri B:&nbsp;
              <bdo className="valueTag">{Nutri_B_left}</bdo> &nbsp;(ml)
            </h1>
          </div>

          <div className="circular">
            <CircularProgressbar
              className="Acid-cir"
              value={Acid_left}
              maxValue={Acid_So_full}
              text={`${Acid_percent}%`}
              styles={buildStyles({
                pathTransitionDuration: 0.5,
                pathColor: `#ffb31a`,
                backgroundColor: "#3e98c7",
              })}
            />
            <h1>
              Acid:&nbsp;
              <bdo className="valueTag">{Acid_left}</bdo> &nbsp;(ml)
            </h1>
          </div>

          <div className="circular">
            <CircularProgressbar
              className="Base-cir"
              value={Base_left}
              maxValue={Base_So_full}
              text={`${Base_percent}%`}
              styles={buildStyles({
                pathColor: `#7300e6`,
                pathTransitionDuration: 0.5,
                backgroundColor: "#3e98c7",
              })}
            />
            <h1>
              Base:&nbsp;
              <bdo className="valueTag">{Base_left}</bdo> &nbsp;(ml)
            </h1>
          </div>
        </div>
      </main>
      <div className="right">
        <div className="top">
          <button id="menu-btn1">
            <svg
              xmlns="http://www.w3.org/2000/svg"
              width="16"
              height="16"
              fill="currentColor"
              className="bi bi-list"
              viewBox="0 0 16 16"
            >
              <path
                fillRule="evenodd"
                d="M2.5 12a.5.5 0 0 1 .5-.5h10a.5.5 0 0 1 0 1H3a.5.5 0 0 1-.5-.5zm0-4a.5.5 0 0 1 .5-.5h10a.5.5 0 0 1 0 1H3a.5.5 0 0 1-.5-.5zm0-4a.5.5 0 0 1 .5-.5h10a.5.5 0 0 1 0 1H3a.5.5 0 0 1-.5-.5z"
              />
            </svg>
          </button>
        </div>
        <div className="userPart">
          <div className="user">
            <svg
              xmlns="http://www.w3.org/2000/svg"
              width="50"
              height="50"
              fill="currentColor"
              className="user-icon"
              viewBox="0 0 16 16"
            >
              <path d="M11 6a3 3 0 1 1-6 0 3 3 0 0 1 6 0z" />
              <path
                fillRule="evenodd"
                d="M0 8a8 8 0 1 1 16 0A8 8 0 0 1 0 8zm8-7a7 7 0 0 0-5.468 11.37C3.242 11.226 4.805 10 8 10s4.757 1.225 5.468 2.37A7 7 0 0 0 8 1z"
              />
            </svg>
            <div className="userName">Welcome {username}</div>
          </div>
          <div className="userDevice">
            <div>My Devices</div>

            <div className="devices">
              <div className="dosingmachine">
                <img src={dosingmachine} className="icon-device" />
                <div className="status-circle" style={state_style}></div>
                <div className="string">
                  Dosing <br />
                  Machine
                </div>
                <h2>State : {state}</h2>
              </div>
            </div>
          </div>
        </div>
        <div className="img">
          <img src={image} />
        </div>
      </div>
    </>
  );
};
export default Dashboard;
