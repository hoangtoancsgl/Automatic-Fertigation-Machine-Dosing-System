import { createContext, useReducer, useState } from "react";
import { deviceReducer } from "../reducers/deviceReducer";
import {
  url,
  DEVICE_LOADED_FAIL,
  DEVICE_LOADED_SUCCESS,
  ADD_DEVICE,
} from "./constants";
import axios from "axios";

export const DeviceContext = createContext();

const DeviceContextProvider = ({ children }) => {
  //state
  const [deviceState, dispatchDevice] = useReducer(deviceReducer, {
    deviceData: [],
  });

  //get device

  const getDevice = async () => {
    try {
      const responce = await axios.get(`${url}/device`);
      if (responce.data.success) {
        dispatchDevice({
          type: DEVICE_LOADED_SUCCESS,
          payload: responce.data.getalldata,
        });
      }
    } catch (error) {
      dispatchDevice({ type: DEVICE_LOADED_FAIL });
    }
  };

  const addDevices = async (newDevice) => {
    try {
      const response = await axios.post(`${url}/device`, newDevice);
      if (response.data.success) {
        dispatchDevice({
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

  const deviceContextData = {
    addDevices,
    getDevice,
    deviceState,
  };
  return (
    <DeviceContext.Provider value={deviceContextData}>
      {children}
    </DeviceContext.Provider>
  );
};
export default DeviceContextProvider;
