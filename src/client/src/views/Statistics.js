import React from "react";
import "./statistics.css";
import axios from "axios";
import ReactApexChart from "react-apexcharts";
import Select from "react-select";
import { url } from "../contexts/constants";
import { useState, useEffect, useContext } from "react";
import { DeviceContext } from "../contexts/DeviceContext";

const Statistics = () => {
  const [selectedDevice, setSelectedDevice] = useState("Device 1");
  const [labelDevice, setLabelDevice] = useState("Device 1");
  const {
    deviceState: { deviceData },
    getDevice,
  } = useContext(DeviceContext);
  // //get last data

  const [chartData, setChartData] = useState({
    seriesTemperature: [
      {
        name: "Temperature",
        data: [],
      },
    ],
    optionsTemperature: {
      chart: {
        id: "area",
        group: "temp",
        type: "area",
      },
      colors: ["#00E396"],
      xaxis: {
        type: "datetime",
        categories: [],
      },
      noData: {
        text: "Loading...",
      },
    },
    seriesPHChart: [
      {
        name: "PH",
        data: [],
      },
    ],
    optionsPhChart: {
      chart: {
        id: "ig",
        group: "ph",
        type: "area",
      },
      colors: ["#008FFB"],
      xaxis: {
        type: "datetime",
        categories: [],
        tickAmount: 6,
      },
      noData: {
        text: "Loading...",
      },
    },

    seriesTDSChart: [
      {
        name: "TDS",
        data: [],
      },
    ],
    optionsTDSChart: {
      chart: {
        id: "li",
        group: "tds",
        type: "area",
      },
      colors: ["#b37700"],
      xaxis: {
        type: "datetime",
        categories: [],
        tickAmount: 6,
      },
      noData: {
        text: "Loading...",
      },
    },
  });

  useEffect(() => {
    getDevice();
  }, []);

  var optionsDevice = [];
  for (let i = 0; i < deviceData.length; i++) {
    let value = [];
    value = { value: deviceData[i].device, label: `Device ${i + 1}` };
    optionsDevice.push(value);
  }

  for (let i = 0; i < deviceData.length; i++) {
    if (selectedDevice === optionsDevice[i].label) {
      setSelectedDevice(deviceData[i].device);
    }
  }
  console.log(selectedDevice);
  const OnChangeDevice = (event) => {
    setSelectedDevice(event.label);
  };

  const [time, setTime] = useState("1");

  function fetchChartData(device, length) {
    axios
      .get(`${url}/data/chart/${device}/${length}`)
      .then((response) => {
        const data = response.data;
        if (data.getalldata[0].length !== 0) {
          setChartData({
            seriesTemperature: [
              {
                name: "Temperature",
                data: data.getalldata[0],
              },
            ],
            optionsTemperature: {
              chart: {
                id: "area",
                group: "temp",
                type: "area",
              },
              colors: ["#00E396"],
              xaxis: {
                type: "datetime",
                categories: data.getalldata[3],
              },
            },
            seriesPHChart: [
              {
                name: "PH",
                data: data.getalldata[2],
              },
            ],
            optionsPhChart: {
              chart: {
                id: "ig",
                group: "ph",
                type: "area",
              },
              colors: ["#008FFB"],
              xaxis: {
                type: "datetime",
                categories: data.getalldata[3],
              },
            },
            seriesTDSChart: [
              {
                name: "TDS",
                data: data.getalldata[1],
              },
            ],
            optionsTDSChart: {
              chart: {
                id: "li",
                group: "tds",
                type: "area",
              },
              colors: ["#b37700"],
              xaxis: {
                type: "datetime",
                categories: data.getalldata[3],
              },
            },
          });
        } else {
          setChartData({
            seriesTemperature: [
              {
                name: "Temperature",
                data: [],
              },
            ],
            optionsTemperature: {
              chart: {
                id: "area",
                group: "temp",
                type: "area",
              },
              colors: ["#00E396"],
              xaxis: {
                type: "datetime",
                categories: [],
              },
            },
            seriesPHChart: [
              {
                name: "PH",
                data: [],
              },
            ],
            optionsPhChart: {
              chart: {
                id: "ig",
                group: "ph",
                type: "area",
              },
              colors: ["#008FFB"],
              xaxis: {
                type: "datetime",
                categories: [],
              },
            },
            seriesTDSChart: [
              {
                name: "TDS",
                data: [],
              },
            ],
            optionsTDSChart: {
              chart: {
                id: "li",
                group: "tds",
                type: "area",
              },
              colors: ["#b37700"],
              xaxis: {
                type: "datetime",
                categories: [],
              },
            },
          });
        }
      })
      .catch((e) => {
        console.log("Error retrieving data!!!");
      });
  }

  useEffect(() => {
    if (time === "1") {
      fetchChartData(selectedDevice, 48);
    } else if (time === "2") {
      fetchChartData(selectedDevice, 336);
    } else if (time === "3") {
      fetchChartData(selectedDevice, 1440);
    }
  }, [selectedDevice, time]);

  const timeOptions = [
    { value: "1", label: "1 day" },
    { value: "2", label: "1 week" },
    { value: "3", label: "1 month" },
  ];

  const onChange = (event) => setTime(event.value);

  return (
    <>
      <div id="wrapper" className="wrapper">
        <div className="select">
          <Select
            options={optionsDevice}
            className="selecttime"
            placeholder={<div>{labelDevice}</div>}
            onChange={OnChangeDevice}
          />
          <Select
            options={timeOptions}
            className="selecttime"
            placeholder={<div>1 day</div>}
            onChange={onChange}
          />
        </div>

        <div className="chart-time-series">
          <div id="chart-small" className="timeseries">
            <ReactApexChart
              options={chartData.optionsPhChart}
              series={chartData.seriesPHChart}
              type="area"
              height={170}
              width={1040}
            />
            <h3>PH Time Series Chart</h3>
          </div>
          <div id="chart-small2" className="timeseries">
            <ReactApexChart
              options={chartData.optionsTDSChart}
              series={chartData.seriesTDSChart}
              type="area"
              height={170}
              width={1040}
            />
            <h3>TDS Time Series Chart</h3>
          </div>

          <div>
            <div id="chart-area" className="timeseries">
              <ReactApexChart
                options={chartData.optionsTemperature}
                series={chartData.seriesTemperature}
                type="area"
                height={170}
                width={1040}
              />
              <h3>Temperature Time Series Chart</h3>
            </div>
          </div>
        </div>
      </div>
      <div className="wrapper"></div>
    </>
  );
};

export default Statistics;
