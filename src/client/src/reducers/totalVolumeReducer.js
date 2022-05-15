import {
  TOTALVOLUME_LOADED_SUCCESS,
  TOTALVOLUME_LOADED_FAIL,
} from "../contexts/constants";
export const totalVolumeReducer = (volume, action) => {
  const { type, payload } = action;
  switch (type) {
    case TOTALVOLUME_LOADED_SUCCESS:
      return {
        ...volume,
        totalVolume: payload,
        totalVolumeLoading: false,
      };
    case TOTALVOLUME_LOADED_FAIL:
      return {
        ...volume,
        totalVolume: [],
        totalVolumeLoading: false,
      };
    default:
      return volume;
  }
};
