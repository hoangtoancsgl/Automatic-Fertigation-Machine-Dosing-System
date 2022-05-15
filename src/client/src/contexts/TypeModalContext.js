import { createContext, useReducer } from "react";
import { typeModalReducer } from "../reducers/typeModalReducer";

import {
  url,
  TYPEMODAL_LOADED_FAIL,
  TYPEMODAL_LOADED_SUCCESS,
  FIND_CONFIGTYPEMODAL,
} from "./constants";
import axios from "axios";
export const TypeModalContext = createContext();

const TypeModalContextProvider = ({ children }) => {
  const [typeModalState, dispatchTypeModal] = useReducer(typeModalReducer, {
    configtypemodal: null,
    typeModal: [],
  });

  // get config type of Crops
  const getTypeModal = async () => {
    try {
      const responce = await axios.get(`${url}/typemodal`);
      if (responce.data.success) {
        dispatchTypeModal({
          type: TYPEMODAL_LOADED_SUCCESS,
          payload: responce.data.getLastConfigType,
        });
      }
    } catch (error) {
      dispatchTypeModal({ type: TYPEMODAL_LOADED_FAIL });
    }
  };
  const findConfigTypeModal = (configTypeModalId) => {
    const configtypemodal = typeModalState.typeModal.find(
      (configtypemodal) => configtypemodal._id === configTypeModalId
    );
    dispatchTypeModal({ type: FIND_CONFIGTYPEMODAL, payload: configtypemodal });
  };

  const typeModalContextData = {
    typeModalState,
    getTypeModal,
    findConfigTypeModal,
  };
  return (
    <TypeModalContext.Provider value={typeModalContextData}>
      {children}
    </TypeModalContext.Provider>
  );
};
export default TypeModalContextProvider;
