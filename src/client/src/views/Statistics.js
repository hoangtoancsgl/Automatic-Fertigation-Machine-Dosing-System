import React from "react";
import "./statistics.css";
import axios from "axios";
import ReactApexChart from "react-apexcharts";
import Select from "react-select";
import { url } from "../contexts/constants";
import { useState } from "react";

const Statistics = () => {
  const arrayTemp = [];
  const arrayTDS = [];
  const arrayPH = [];
  const arrayTime = [];
  let seriesTemp = [];
  let seriesTDS = [];
  let seriesPH = [];

  const [time, setTime] = useState("1");
  if (time === "1") {
    fetchChartData(48);
  } else if (time === "2") {
    fetchChartData(336);
  } else if (time === "3") {
    fetchChartData(1440);
  }
  function fetchChartData(length) {
    axios
      .get(`${url}/data/chart`)
      .then((response) => {
        const data = response.data;

        for (let j = 0; j < length; j++) {
          var date = new Date(data.getalldata[j].createdAt);
          arrayTime.push(date.getTime());
          arrayTemp.push(data.getalldata[j].temperature * 1);
          arrayTDS.push(data.getalldata[j].TDS * 1);
          arrayPH.push(data.getalldata[j].PH * 1);
        }
        var values = [arrayTemp, arrayTDS, arrayPH, arrayTime];
        var i = 0;
        while (i < length) {
          seriesTemp.push([values[3][i], values[0][i]]);
          seriesTDS.push([values[3][i], values[1][i]]);
          seriesPH.push([values[3][i], values[2][i]]);
          i++;
        }
        window.dispatchEvent(new Event("resize"));
      })
      .catch(() => {
        console.log("Error retrieving data!!!");
      });
  }

  const seriesArea = [
    {
      name: "Temperature",
      data: seriesTemp,
    },
  ];
  const optionsArea = {
    chart: {
      id: "area",
      group: "social",
      type: "area",
    },
    colors: ["#00E396"],
    xaxis: {
      type: "datetime",
      min: arrayTime[0],
    },
    zoom: {
      type: "x",
      enabled: true,
      autoScaleYaxis: true,
    },
    toolbar: {
      autoSelected: "zoom",
    },
  };

  const seriesSmall = [
    {
      name: "PH",
      data: seriesPH,
    },
  ];
  const optionsSmall = {
    chart: {
      id: "ig",
      group: "social",
      type: "area",
    },
    colors: ["#008FFB"],
    xaxis: {
      type: "datetime",
      min: arrayTime[0],
      tickAmount: 6,
    },
    zoom: {
      type: "x",
      enabled: true,
      autoScaleYaxis: true,
    },
    toolbar: {
      autoSelected: "zoom",
    },
  };

  const seriesSmall2 = [
    {
      name: "TDS",
      data: seriesTDS,
    },
  ];
  const optionsSmall2 = {
    chart: {
      id: "li",
      group: "social",
      type: "area",
    },
    colors: ["#b37700"],
    xaxis: {
      type: "datetime",
      min: arrayTime[0],
      tickAmount: 6,
    },
    zoom: {
      type: "x",
      enabled: true,
      autoScaleYaxis: true,
    },
    toolbar: {
      autoSelected: "zoom",
    },
  };
  const options = [
    { value: "1", label: "1 day" },
    { value: "2", label: "1 week" },
    { value: "3", label: "1 month" },
  ];

  const onChange = (event) => setTime(event.value);

  return (
    <>
      <div id="wrapper" className="wrapper">
        <Select
          options={options}
          className="selecttime"
          placeholder={<div>1 day</div>}
          onChange={onChange}
        />
        <div className="chart-time-series">
          <div id="chart-small" className="timeseries">
            <ReactApexChart
              options={optionsSmall}
              series={seriesSmall}
              type="area"
              height={170}
              width={1040}
            />
            <h3>PH Time Series Chart</h3>
          </div>
          <div id="chart-small2" className="timeseries">
            <ReactApexChart
              options={optionsSmall2}
              series={seriesSmall2}
              type="area"
              height={170}
              width={1040}
            />
            <h3>TDS Time Series Chart</h3>
          </div>

          <div>
            <div id="chart-area" className="timeseries">
              <ReactApexChart
                options={optionsArea}
                series={seriesArea}
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
