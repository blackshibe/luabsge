using System;
using UnityEngine;
using UnityEngine.Rendering;
using UnityEngine.Rendering.Universal;
using UnityEngine.TestTools;


[Serializable]
public struct PassSettings
{
    [SerializeField]
    public bool enableInSceneView;
}

public class RaytracerBlitRenderFeature : ScriptableRendererFeature
{
    [SerializeField] private PassSettings settings = new PassSettings { enableInSceneView = true };



    private sealed class Pass : ScriptableRenderPass
    {

        ProfilingSampler m_ProfilingSampler = new ProfilingSampler("RayMarching Pass");
        Material m_Material;
        public PassSettings m_settings;
        RTHandle m_CameraColorTarget;

        public Pass(Material material, PassSettings settings)
        {
            m_Material = material;
            m_settings = settings;
            renderPassEvent = RenderPassEvent.BeforeRenderingPostProcessing;
        }

        public void SetTarget(RTHandle colorHandle)
        {
            m_CameraColorTarget = colorHandle;
        }

        [System.Obsolete]
        public override void OnCameraSetup(CommandBuffer cmd, ref RenderingData renderingData)
        {
            ConfigureTarget(m_CameraColorTarget);
        }

        [System.Obsolete]
        public override void Execute(ScriptableRenderContext context, ref RenderingData renderingData)
        {
            if (renderingData.cameraData.cameraType != CameraType.Game && !m_settings.enableInSceneView) return;

            m_Material.SetMatrix("_CameraInvProj", renderingData.cameraData.camera.projectionMatrix.inverse);
            m_Material.SetMatrix("_CameraToWorld", renderingData.cameraData.camera.cameraToWorldMatrix);
            m_Material.SetVector("_CameraPosition", renderingData.cameraData.camera.transform.position);

            // if (m_Material == null) return;
            CommandBuffer cmd = CommandBufferPool.Get();
            using (new ProfilingScope(cmd, m_ProfilingSampler))
            {
                // m_Material.SetColor("_BaseColor", param.baseColor.value);
                Blitter.BlitCameraTexture(cmd, m_CameraColorTarget, m_CameraColorTarget, m_Material, 0);
            }

            context.ExecuteCommandBuffer(cmd);
            cmd.Clear();

            CommandBufferPool.Release(cmd);
        }
    }

    Pass m_RenderPass;
    Shader m_Shader;
    Material m_Material;

    public override void AddRenderPasses(ScriptableRenderer renderer, ref RenderingData renderingData)
    {
        renderer.EnqueuePass(m_RenderPass);
    }

    [System.Obsolete]
    public override void SetupRenderPasses(ScriptableRenderer renderer, in RenderingData renderingData)
    {
        // Calling ConfigureInput with the ScriptableRenderPassInput.Color argument
        // ensures that the opaque texture is available to the Render Pass.
        m_RenderPass.ConfigureInput(ScriptableRenderPassInput.Color);
        m_RenderPass.SetTarget(renderer.cameraColorTargetHandle);
    }

    public override void Create()
    {
        m_Shader = Shader.Find("Custom/NewBlitScriptableRenderPipelineShader");
        m_Material = CoreUtils.CreateEngineMaterial(m_Shader);
        m_RenderPass = new Pass(m_Material, settings);
    }

    protected override void Dispose(bool disposing)
    {
        CoreUtils.Destroy(m_Material);
    }

    private void Update()
    {
        if (m_RenderPass != null)
        {
            m_RenderPass.m_settings = settings;
        }
    }
}