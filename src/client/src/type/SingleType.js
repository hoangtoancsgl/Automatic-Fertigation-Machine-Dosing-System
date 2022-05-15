import ActionButtons from "./ActionButton";
import "../views/views.css";
const SingleType = ({
  config: { _id, type, TDS, dead_TDS, PH, dead_PH, nutri_Ratio },
}) => (
  <div className="singleType">
    <div className="value">
      <div className="type">Type: {type}</div>
      <div className="TDS">TDS: {TDS}</div>
      <div className="dead_TDS">dead_TDS: {dead_TDS}</div>
      <div className="PH">PH: {PH}</div>
      <div className="dead_PH">dead_PH: {dead_PH}</div>
      <div className="nutri_ratio">Nutri Ratio: {nutri_Ratio}</div>
    </div>

    <ActionButtons
      _id={_id}
      type={type}
      TDS={TDS}
      dead_TDS={dead_TDS}
      PH={PH}
      dead_PH={dead_PH}
      nutri_Ratio={nutri_Ratio}
      className="actionButton"
    />
  </div>
);

export default SingleType;
