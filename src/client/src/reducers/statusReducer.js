import {
  STATUS_LOADED_FAIL,
  STATUS_LOADED_SUCCESS,
} from "../contexts/constants";
export const statusReducer = (status, action) => {
  const { type, payload } = action;
  switch (type) {
    case STATUS_LOADED_SUCCESS:
      return {
        ...status,
        status: payload,
        statusLoading: false,
      };
    case STATUS_LOADED_FAIL:
      return {
        ...status,
        status: [],
        statusLoading: false,
      };
    default:
      return status;
  }
};
