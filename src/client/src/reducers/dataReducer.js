import {
  DATA_LOADED_FAIL,
  DATA_LOADED_SUCCESS,
  ADD_DEVICE,
} from "../contexts/constants";
export const dataReducer = (state, action) => {
  const { type, payload } = action;
  switch (type) {
    case DATA_LOADED_SUCCESS:
      return {
        ...state,
        data: payload,
        dataLoading: false,
      };
    case DATA_LOADED_FAIL:
      return {
        ...state,
        data: [],
        dataLoading: false,
      };
    case ADD_DEVICE:
      return {
        ...state,
      };

    default:
      return state;
  }
};
