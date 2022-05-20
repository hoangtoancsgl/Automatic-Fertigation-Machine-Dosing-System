import {
  DEVICE_LOADED_FAIL,
  DEVICE_LOADED_SUCCESS,
  ADD_DEVICE,
} from "../contexts/constants";
export const deviceReducer = (devic, action) => {
  const { type, payload } = action;
  switch (type) {
    case DEVICE_LOADED_SUCCESS:
      return {
        ...devic,
        deviceData: payload,
      };
    case DEVICE_LOADED_FAIL:
      return {
        ...devic,
        deviceData: [],
      };
    case ADD_DEVICE:
      return {
        ...devic,
        deviceData: [...devic.deviceData, payload],
      };

    default:
      return devic;
  }
};
