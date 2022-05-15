import {
  SETVOLUME_LOADED_FAIL,
  SETVOLUME_LOADED_SUCCESS,
  ADD_SETVOLUME,
  FIND_SETVOLUME,
  UPDATE_SETVOLUME,
} from "../contexts/constants";
export const setVolumeReducer = (setvolume, action) => {
  const { type, payload } = action;
  switch (type) {
    case SETVOLUME_LOADED_SUCCESS:
      return {
        ...setvolume,
        setVolume: payload,
        setVolumeLoading: false,
      };
    case SETVOLUME_LOADED_FAIL:
      return {
        ...setvolume,
        setVolume: [],
        setVolumeLoading: false,
      };
    case ADD_SETVOLUME:
      return {
        ...setvolume,
        setVolume: [...setvolume.setVolume, payload],
      };

    // case FIND_CONFIGTYPE:
    //   return { ...setvolume, configtype: payload };

    // case UPDATE_CONFIGTYPE:
    //   const newType = typeagr.config.map((configtype) =>
    //     configtype._id === payload._id ? payload : configtype
    //   );

    //   return {
    //     ...typeagr,
    //     config: newType,
    //   };

    default:
      return setvolume;
  }
};
