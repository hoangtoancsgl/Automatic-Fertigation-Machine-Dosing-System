import {
  CONFIGDATA_LOADED_FAIL,
  CONFIGDATA_LOADED_SUCCESS,
  ADD_CONFIGDATA,
} from "../contexts/constants";
export const configReducer = (config, action) => {
  const { configData, payload } = action;
  switch (configData) {
    case CONFIGDATA_LOADED_SUCCESS:
      return {
        ...config,
        currentConfig: payload,
      };
    case CONFIGDATA_LOADED_FAIL:
      return {
        ...config,
        currentConfig: [],
      };
    case ADD_CONFIGDATA:
      return {
        ...config,
        // currentConfig: [...config.currentConfig, payload],
      };
    default:
      return config;
  }
};
