using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class DoubleSpiral : MonoBehaviour
{
    
    public float projectileSpeed;
    public GameObject ProjectilePrefab;
    public float angle = 0f;

    private float dt;
    private Vector3 startPoint;
    // Start is called before the first frame update

    // Update is called once per frame
    void Update()
    {
        dt += Time.deltaTime;

        if (dt >= 0.1f)
        {
            startPoint = transform.position;
            doubleSpiral();
            dt = 0;
        }
    }

    private void doubleSpiral()
    {
        for (int i = 0; i <= 1; i++)
        {
            // Direction Calculation

            float projectileDirXPosition = startPoint.x + Mathf.Sin(((angle + 180f * i) * Mathf.PI) / 180f);
            float projectileDirYPosition = startPoint.y + Mathf.Cos(((angle + 180f * i) * Mathf.PI) / 180f);

            Vector3 projectileVector = new Vector3(projectileDirXPosition, projectileDirYPosition, 0);
            Vector3 projectileMoveDirection = (projectileVector - startPoint).normalized * projectileSpeed;

            GameObject tmpObj = Instantiate(ProjectilePrefab, startPoint, Quaternion.identity);
            tmpObj.GetComponent<Rigidbody>().velocity = new Vector3(projectileMoveDirection.x, 0, projectileMoveDirection.y);
            
        }

        angle += 10f;

        if(angle >= 360f)
        {
            angle = 0f;
        }

    }
}
