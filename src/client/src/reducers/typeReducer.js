import {
  CONFIGTYPE_LOADED_SUCCESS,
  CONFIGTYPE_LOADED_FAIL,
  ADD_CONFIGTYPE,
  DELETE_CONFIGTYPE,
  FIND_CONFIGTYPE,
  UPDATE_CONFIGTYPE,
} from "../contexts/constants";
export const typeReducer = (typeagr, action) => {
  const { type, payload } = action;
  switch (type) {
    case CONFIGTYPE_LOADED_SUCCESS:
      return {
        ...typeagr,
        config: payload,
        configLoading: false,
      };
    case CONFIGTYPE_LOADED_FAIL:
      return {
        ...typeagr,
        config: [],
        configLoading: false,
      };
    case ADD_CONFIGTYPE:
      return {
        ...typeagr,
        config: [...typeagr.config, payload],
      };

    case DELETE_CONFIGTYPE:
      return {
        ...typeagr,
        config: typeagr.config.filter((config) => config._id !== payload),
      };

    case FIND_CONFIGTYPE:
      return { ...typeagr, configtype: payload };

    case UPDATE_CONFIGTYPE:
      const newType = typeagr.config.map((configtype) =>
        configtype._id === payload._id ? payload : configtype
      );

      return {
        ...typeagr,
        config: newType,
      };

    default:
      return typeagr;
  }
};
