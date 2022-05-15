import Modal from "react-bootstrap/Modal";
import Form from "react-bootstrap/Form";
import Button from "react-bootstrap/Button";
import "bootstrap/dist/css/bootstrap.min.css";
import { useContext, useEffect, useState } from "react";
import { TypeContext } from "../contexts/TypeContext";

const UpdateTypeModal = () => {
  const {
    typeState: { configtype },
    showUpdateTypeModal,
    setShowUpdateTypeModal,
    updateConfigType,
    setShowToast,
  } = useContext(TypeContext);

  const [updateType, setUpdateType] = useState(configtype);

  useEffect(() => setUpdateType(configtype), [configtype]);

  const { type, TDS, dead_TDS, PH, dead_PH, nutri_Ratio } = updateType;
  const onChangeUpdatedTypeForm = (event) =>
    setUpdateType({ ...updateType, [event.target.name]: event.target.value });

  const closeDialog = () => {
    setShowUpdateTypeModal(false);
  };

  const onSubmit = async (event) => {
    event.preventDefault();
    const { success, message } = await updateConfigType(updateType);
    setShowUpdateTypeModal(false);
    setShowToast({ show: true, message, Type: success ? "info" : "danger" });
  };

  return (
    <Modal show={showUpdateTypeModal} onHide={closeDialog}>
      <Modal.Header closeButton>
        <Modal.Title>Any Change?</Modal.Title>
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
              onChange={onChangeUpdatedTypeForm}
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
              onChange={onChangeUpdatedTypeForm}
            />
          </Form.Group>
          <Form.Group className="addType">
            <Form.Control
              type="number"
              step="0.01"
              placeholder="TDS dead value"
              name="dead_TDS"
              value={dead_TDS}
              onChange={onChangeUpdatedTypeForm}
            />
          </Form.Group>
          <Form.Group className="addType">
            <Form.Control
              type="number"
              step="0.01"
              placeholder="PH value"
              name="PH"
              value={PH}
              onChange={onChangeUpdatedTypeForm}
            />
          </Form.Group>
          <Form.Group className="addType">
            <Form.Control
              type="number"
              step="0.01"
              placeholder="PH dead value"
              name="dead_PH"
              value={dead_PH}
              onChange={onChangeUpdatedTypeForm}
            />
          </Form.Group>
          <Form.Group className="addType">
            <Form.Control
              type="number"
              step="0.01"
              placeholder="Nutri ratio value"
              name="nutri_Ratio"
              value={nutri_Ratio}
              onChange={onChangeUpdatedTypeForm}
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

export default UpdateTypeModal;
