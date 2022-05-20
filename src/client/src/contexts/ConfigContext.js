import { configReducer } from "../reducers/configReducer";
import { createContext, useReducer } from "react";
import {
  url,
  ADD_CONFIGDATA,
  CONFIGDATA_LOADED_SUCCESS,
  CONFIGDATA_LOADED_FAIL,
} from "./constants";
import axios from "axios";

export const ConfigContext = createContext();

const ConfigContextProvider = ({ children }) => {
  const [configState, dispatchConfig] = useReducer(configReducer, {
    currentConfig: [],
  });
  // config data
  const configManual = async (newConfig) => {
    try {
      const response = await axios.post(`${url}/configdata`, newConfig);
      if (response.data.success) {
        dispatchConfig({
          type: ADD_CONFIGDATA,
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

  //get current config
  const getConfigData = async (device) => {
    try {
      const responce = await axios.get(`${url}/configdata/${device}`);
      if (responce.data.success) {
        dispatchConfig({
          type: CONFIGDATA_LOADED_SUCCESS,
          payload: responce.data.getLastConfigData,
        });
      }
    } catch (error) {
      dispatchConfig({ type: CONFIGDATA_LOADED_FAIL });
    }
  };
  const configContextData = {
    configManual,
    getConfigData,
    configState,
  };
  return (
    <ConfigContext.Provider value={configContextData}>
      {children}
    </ConfigContext.Provider>
  );
};
export default ConfigContextProvider;
