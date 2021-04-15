using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class Homing : MonoBehaviour
{
    public Transform rocketTarget;
    public Rigidbody rocketRigidBody;

    public float turn;
    public float rocketVelocity;

    private void FixedUpdate()
    {
        rocketRigidBody.velocity = transform.forward * rocketVelocity;

        var rocketTargetRptation = Quaternion.LookRotation(rocketTarget.position - transform.position);

        rocketRigidBody.MoveRotation(Quaternion.RotateTowards(transform.rotation, rocketTargetRptation, turn));
    }
}
