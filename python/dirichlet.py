from scipy.special import gamma, gammaln
import operator
import math
from logsumexp import logsumexp

def product(l):
    return reduce(operator.mul, l)

def beta(alphas):
    """Multivariate beta function"""
    return math.exp(sum(map(gammaln, alphas)) - gammaln(sum(alphas)))

def dirichlet(probs, obs, s=1):
    """Dirichlet probability density for a given set probabilities and prior
    observation counts.  s serves as a concentration parameter between 0 and 1,
    with smaller s yielding progressively more diffuse probability density
    distributions."""
    alphas = [(a + 1) * float(s) for a in obs]
    return 1 / beta(alphas) * product([math.pow(x, a - 1) for x, a in zip(probs, alphas)])

def dirichlet_maximum_likelihood_ratio(probs, obs, s=1):
    """Ratio between the dirichlet for the specific probs and obs and the
    maximum likelihood value for the dirichlet over the given probs (this can
    be determined by partitioning the observations according to the
    probabilities)"""
    maximum_likelihood = dirichlet(probs, [float(sum(obs)) / len(obs) for o in obs], s)
    return dirichlet(probs, obs, s) / float(maximum_likelihood)

def multinomial(probs, obs):
    return math.factorial(sum(obs)) / product(map(math.factorial, obs)) * product([math.pow(p, x) for p,x in zip(probs, obs)])

def multinomial_dirichlet(probs, obs): return multinomial(probs, obs) * dirichlet(probs, obs)

def binomial(successes, trials, prob):
    return math.factorial(trials) / (math.factorial(successes) * math.factorial(trials - successes)) * math.pow(prob, successes) * math.pow(1 - prob, trials - successes)
