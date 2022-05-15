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

  // const [showAddTypeModal, setShowAddTypeModal] = useState(false);
  // const [showUpdateTypeModal, setShowUpdateTypeModal] = useState(false);
  // const [showToast, setShowToast] = useState({
  //   show: false,
  //   message: "",
  //   type: null,
  // });
  //get data
  const getData = async () => {
    try {
      const responce = await axios.get(`${url}/data`);
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

  const addDevices = async (newDevice) => {
    try {
      const response = await axios.post(`${url}/device`, newDevice);
      if (response.data.success) {
        dispatch({
          type: ADD_DEVICE,
          payload: response.data.post,
        });
        return response.data;
      }
    } catch (error) {
      return error.response.data
        ? error.response.data
        : { success: false, message: "Server error" };
    }
  };

  const dataContextData = {
    dataState,
    addDevices,
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
