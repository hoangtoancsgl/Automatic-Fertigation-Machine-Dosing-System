import { createContext, useReducer, useState } from "react";
import { dataReducer } from "../reducers/dataReducer";
import {
  url,
  DATA_LOADED_FAIL,
  DATA_LOADED_SUCCESS,
  ADD_DEVICE,
  STATE_LOADED_FAIL,
  STATE_LOADED_SUCCESS,
} from "./constants";
import axios from "axios";

export const DataContext = createContext();

const DataContextProvider = ({ children }) => {
  //state
  const [dataState, dispatch] = useReducer(dataReducer, {
    data: [],
    dataLoading: true,
  });

  //get data
  const getData = async (device) => {
    try {
      const responce = await axios.get(`${url}/data/${device}`);
      if (responce.data.success) {
        dispatch({
          type: DATA_LOADED_SUCCESS,
          payload: responce.data.getalldata,
        });
      }
    } catch (error) {
      dispatch({ type: DATA_LOADED_FAIL });
    }
  };

  const getChartData = async () => {
    try {
      const responce = await axios.get(`${url}/data/chart`);
      if (responce.data.success) {
        dispatch({
          type: DATA_LOADED_SUCCESS,
          payload: responce.data.getalldata,
        });
      }
    } catch (error) {
      dispatch({ type: DATA_LOADED_FAIL });
    }
  };

  const dataContextData = {
    dataState,
    getData,
    getChartData,
  };
  return (
    <DataContext.Provider value={dataContextData}>
      {children}
    </DataContext.Provider>
  );
};
export default DataContextProvider;
