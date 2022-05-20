import {
  CONFIGDATA_LOADED_FAIL,
  CONFIGDATA_LOADED_SUCCESS,
  ADD_CONFIGDATA,
} from "../contexts/constants";
export const configReducer = (configdata, action) => {
  const { type, payload } = action;
  switch (type) {
    case CONFIGDATA_LOADED_SUCCESS:
      return {
        ...configdata,
        currentConfig: payload,
      };
    case CONFIGDATA_LOADED_FAIL:
      return {
        ...configdata,
        currentConfig: [],
      };
    case ADD_CONFIGDATA:
      return {
        ...configdata,
        currentConfig: payload,
      };
    default:
      return configdata;
  }
};
