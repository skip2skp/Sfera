############# INIT ###############

#import what I need
import numpy as np
import matplotlib.pyplot as plt

m_e=551 #keV #electron mass
E_init=E_Cs=661.7 #keV #nominal energy of photon emitted by Cs source

N_photons=10000 #number of incident photons
prob=0.5 #ratio of photons that scatter twice in the crystal

#range of definition of d(sigma)/d(E_out) in function of E_in (defaults to E_Cs)
def sample(E=E_init, step=0.5):
    return np.arange(0.1+(m_e*E/(m_e+2*E)), E, step)

#d(sigma)/d(E_out) with e_in (defaults to E_Cs) as a parameter, computed from the Klein-Nishima formula
def crossSection(E_out, E_in=E_init):
    x=np.arccos(m_e/E_in - m_e/E_out + 1)
    return (E_out/E_in)*(E_in/E_out + E_out/E_in - np.sin(x)*np.sin(x))

############### RUN ###################

#single scattering
once=E_init - np.array([np.random.choice(sample(),p=crossSection(sample())/crossSection(sample()).sum()) for i in range (0,N_photons)])

#double scattering
twice=E_init - np.array([np.random.choice(sample(E),p=crossSection(sample(E),E)/crossSection(sample(E),E).sum()) for E in [np.random.choice(sample(),p=crossSection(sample())/crossSection(sample()).sum()) for i in range (0,int(round(N_photons*prob)))]])

#either single or double, with ratio=prob
total=np.append(once, twice)

############ PLOTS #############

# single

plt.hist(once,80)
plt.xlabel("Energy lost in a single compton scattering [keV]")
plt.savefig("/tmp/single_compton.jpg")
plt.clf()

# double

plt.hist(twice,80)
plt.xlabel("Energy lost by photons undergoing Compton scattering twice [keV]")
plt.savefig("/tmp/double_compton.jpg")
plt.clf()

# total

plt.hist(total,80)
plt.xlabel("Energy lost in multiple compton scattering [keV]")
plt.savefig("/tmp/compton.jpg")

print("Saved histograms"+"single_compton.jpg"+"compton_compton.jpg"+"compton.jpg"+"in /tmp/") 
