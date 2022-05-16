import Modal from "react-bootstrap/Modal";
import Form from "react-bootstrap/Form";
import Button from "react-bootstrap/Button";
import "bootstrap/dist/css/bootstrap.min.css";
import { useContext, useState } from "react";
import { TypeContext } from "../contexts/TypeContext";
const AddTypeModal = () => {
  const { showAddTypeModal, setShowAddTypeModal, addConfigtype, setShowToast } =
    useContext(TypeContext);

  const [newType, setNewType] = useState({
    type: "",
    TDS: "",
    dead_TDS: "",
    PH: "",
    dead_PH: "",
    nutri_Ratio: "",
  });

  const { type, TDS, dead_TDS, PH, dead_PH, nutri_Ratio } = newType;

  const onChangeNewTypeForm = (event) =>
    setNewType({ ...newType, [event.target.name]: event.target.value });

  const resetAndAddTypeData = () => {
    setNewType({
      type: "",
      TDS: "",
      dead_TDS: "",
      PH: "",
      dead_PH: "",
      nutri_Ratio: "",
    });
    setShowAddTypeModal(false);
  };
  const closeDialog = () => {
    resetAndAddTypeData();
  };

  const onSubmit = async (event) => {
    event.preventDefault();
    const { success, message } = await addConfigtype(newType);
    resetAndAddTypeData();
    setShowToast({ show: true, message, Type: success ? "info" : "danger" });
  };

  return (
    <Modal show={showAddTypeModal} onHide={closeDialog}>
      <Modal.Header closeButton>
        <Modal.Title>Add new vegetables</Modal.Title>
      </Modal.Header>
      <Form onSubmit={onSubmit}>
        <Modal.Body>
          <Form.Group className="addType">
            <Form.Control
              type="text"
              placeholder="Type"
              name="type"
              required
              aria-describedby="title-help"
              value={type}
              onChange={onChangeNewTypeForm}
            />
            <Form.Text id="title-help" muted>
              Required
            </Form.Text>
          </Form.Group>
          <Form.Group className="addType">
            <Form.Control
              type="number"
              step="0.01"
              placeholder="TDS value"
              name="TDS"
              value={TDS}
              onChange={onChangeNewTypeForm}
            />
          </Form.Group>
          <Form.Group className="addType">
            <Form.Control
              type="number"
              step="0.01"
              placeholder="TDS dead value"
              name="dead_TDS"
              value={dead_TDS}
              onChange={onChangeNewTypeForm}
            />
          </Form.Group>
          <Form.Group className="addType">
            <Form.Control
              type="number"
              step="0.01"
              placeholder="PH value"
              name="PH"
              value={PH}
              onChange={onChangeNewTypeForm}
            />
          </Form.Group>
          <Form.Group className="addType">
            <Form.Control
              type="number"
              step="0.01"
              placeholder="PH dead value"
              name="dead_PH"
              value={dead_PH}
              onChange={onChangeNewTypeForm}
            />
          </Form.Group>
          <Form.Group className="addType">
            <Form.Control
              type="number"
              step="0.01"
              placeholder="Nutri ratio value"
              name="nutri_Ratio"
              value={nutri_Ratio}
              onChange={onChangeNewTypeForm}
            />
          </Form.Group>
        </Modal.Body>
        <Modal.Footer>
          <Button variant="secondary" onClick={closeDialog}>
            Cancel
          </Button>
          <Button variant="primary" type="submit">
            Set
          </Button>
        </Modal.Footer>
      </Form>
    </Modal>
  );
};

export default AddTypeModal;
