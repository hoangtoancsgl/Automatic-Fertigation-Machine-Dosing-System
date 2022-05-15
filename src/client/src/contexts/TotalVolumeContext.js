import { createContext, useReducer } from "react";
import { totalVolumeReducer } from "../reducers/totalVolumeReducer";
import {
  url,
  TOTALVOLUME_LOADED_SUCCESS,
  TOTALVOLUME_LOADED_FAIL,
} from "./constants";
import axios from "axios";
export const TotalVolumeContext = createContext();

const TotalVolumeContextProvider = ({ children }) => {
  const [totalVolumeState, dispatchTotalVolume] = useReducer(
    totalVolumeReducer,
    {
      totalVolume: [],
      totalVolumeLoading: true,
    }
  );
  /// get status of device
  const getTotalVolume = async () => {
    try {
      const responce = await axios.get(`${url}/totalvolume`);
      if (responce.data.success) {
        dispatchTotalVolume({
          type: TOTALVOLUME_LOADED_SUCCESS,
          payload: responce.data.getalldata,
        });
      }
    } catch (error) {
      dispatchTotalVolume({ type: TOTALVOLUME_LOADED_FAIL });
    }
  };

  const totalVolumeContextData = {
    totalVolumeState,
    getTotalVolume,
  };
  return (
    <TotalVolumeContext.Provider value={totalVolumeContextData}>
      {children}
    </TotalVolumeContext.Provider>
  );
};
export default TotalVolumeContextProvider;
