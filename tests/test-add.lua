scene_path=sim.getStringParameter(sim.stringparam_scene_path)
im1=simIM.read(scene_path..'/gray1.png')
im2=simIM.read(scene_path..'/rgb1.png')
im=simIM.add(im1,im2)
simIM.write(im,scene_path..'/test-add.png')
simIM.destroy(im1)
simIM.destroy(im2)
simIM.destroy(im)
print(simIM.numActiveHandles()..' active handles')
