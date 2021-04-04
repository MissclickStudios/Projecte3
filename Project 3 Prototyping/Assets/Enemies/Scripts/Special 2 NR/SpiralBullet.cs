using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class SpiralBullet : MonoBehaviour
{
    public GameObject ProjectilePrefab;
    public float projectileSpeed;

    private float angle = 0f;

    [Header("Private Variables")]
    private Vector3 startPoint;

    private float dt;

    // Update is called once per frame
    void Update()
    {
        dt += Time.deltaTime;

        if (dt >= 0.01f)
        {
            startPoint = transform.position;
            Spiral();
            dt = 0;
        }
    }

    private void Spiral()
    {
        // Direction Calculation

        float projectileDirXPosition = startPoint.x + Mathf.Sin((angle * Mathf.PI) / 180);
        float projectileDirYPosition = startPoint.y + Mathf.Cos((angle * Mathf.PI) / 180);

        Vector3 projectileVector = new Vector3(projectileDirXPosition, projectileDirYPosition, 0);
        Vector3 projectileMoveDirection = (projectileVector - startPoint).normalized * projectileSpeed;

        GameObject tmpObj = Instantiate(ProjectilePrefab, startPoint, Quaternion.identity);
        tmpObj.GetComponent<Rigidbody>().velocity = new Vector3(projectileMoveDirection.x, 0, projectileMoveDirection.y);

        angle += 10;
    }
}
