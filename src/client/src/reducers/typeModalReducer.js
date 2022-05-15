import {
  TYPEMODAL_LOADED_FAIL,
  TYPEMODAL_LOADED_SUCCESS,
  FIND_CONFIGTYPEMODAL,
} from "../contexts/constants";
export const typeModalReducer = (modaltype, action) => {
  const { type, payload } = action;
  switch (type) {
    case TYPEMODAL_LOADED_SUCCESS:
      return {
        ...modaltype,
        typeModal: payload,
      };
    case TYPEMODAL_LOADED_FAIL:
      return {
        ...modaltype,
        typeModal: [],
      };

    case FIND_CONFIGTYPEMODAL:
      return { ...modaltype, configtypemodal: payload };

    default:
      return modaltype;
  }
};
