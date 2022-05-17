import ActionButtons from "./ActionButton";
import nutriRatioImg from "../assets/nutriRatio.png";
import deadPHImg from "../assets/PH_dead.png";
import PHSet from "../assets/PHSet.png";
import TDSSet from "../assets/TDS_img.png";
import deadTDS from "../assets/TDS-deadvalue.png";
import "../views/views.css";
const SingleType = ({
  config: {
    _id,
    type,
    TDS,
    dead_TDS,
    PH,
    dead_PH,
    nutri_Ratio,
    selectedDevice,
  },
}) => {
  return (
    <div className="singleType">
      <div className="value">
        <div className="typeSetting">{type}</div>
        <div className="tds-ph-nutri">
          <div className="TDSSetting">
            <img src={TDSSet}></img>
            <div className="TDS_title"> TDS: {TDS}</div>
          </div>
          <div className="PHSetting">
            <img src={PHSet}></img>
            <div className="PH_title"> PH: {PH}</div>
          </div>
          <div className="nutri_ratioSetting">
            <img src={nutriRatioImg}></img>
            <div className="nutri_ratio_title"> Nutri Ratio: {nutri_Ratio}</div>
          </div>
        </div>
        <div className="dead-tds-ph">
          <div className="dead_TDSSetting">
            <img src={deadTDS}></img>
            <div className="dead_TDS_title">TDS threshold : {dead_TDS}</div>
          </div>
          <div className="dead_PHSetting">
            <img src={deadPHImg}></img>
            <div className="dead_PH_title"> PH threshold: {dead_PH}</div>
          </div>
        </div>
      </div>

      <ActionButtons _id={_id} selectedDevice={selectedDevice} />
    </div>
  );
};

export default SingleType;
