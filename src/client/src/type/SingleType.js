import ActionButtons from "./ActionButton";
import nutriRatioImg from "../assets/nutriRatio.png";
import deadPHImg from "../assets/PH_dead.png";
import PHSet from "../assets/PHSet.png";
import TDSSet from "../assets/TDS_img.png";
import deadTDS from "../assets/TDS-deadvalue.png";
import "../views/views.css";
const SingleType = ({
  config: { _id, type, TDS, dead_TDS, PH, dead_PH, nutri_Ratio },
}) => (
  <div className="singleType">
    <div className="value">
      <div className="typeSetting">{type}</div>
      <div className="tds-ph-nutri">
        <div className="TDSSetting">
          <img src={TDSSet}></img>
          TDS: {TDS}
        </div>
        <div className="PHSetting">
          <img src={PHSet}></img>
          PH: {PH}
        </div>
        <div className="nutri_ratioSetting">
          <img src={nutriRatioImg}></img>
          Nutri Ratio: {nutri_Ratio}
        </div>
      </div>
      <div className="dead-tds-ph">
        <div className="dead_TDSSetting">
          <img src={deadTDS}></img>
          TDS threshold : {dead_TDS}
        </div>
        <div className="dead_PHSetting">
          <img src={deadPHImg}></img>
          PH threshold: {dead_PH}
        </div>
      </div>
    </div>

    <ActionButtons
      _id={_id}
      type={type}
      TDS={TDS}
      dead_TDS={dead_TDS}
      PH={PH}
      dead_PH={dead_PH}
      nutri_Ratio={nutri_Ratio}
    />
  </div>
);

export default SingleType;
